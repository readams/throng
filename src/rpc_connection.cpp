/* -*- C++ -*-; c-basic-offset: 4; indent-tabs-mode: nil */
/*
 * Implementation for rpc connection class.
 *
 * Copyright (c) 2015 Cisco Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You
 * may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rpc_connection.h"
#include "logger.h"

#include "throng_messages.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/connect.hpp>

#include <functional>
#include <chrono>
#include <arpa/inet.h>

namespace throng {
namespace internal {

using std::unique_ptr;
using std::make_shared;
using std::bind;
using std::vector;
using std::chrono::steady_clock;
using boost::system::error_code;
namespace ba = boost::asio;
using ba::io_service;
using ba::steady_timer;
using ba::ip::address;

LOGGER("rpc");

rpc_connection::rpc_connection(ctx_internal& ctx_,
                               uint64_t conn_id_,
                               std::shared_ptr<rpc_handler> handler_,
                               stop_handler_type stop_handler_) :
    ctx(ctx_), conn_id(conn_id_), handler(std::move(handler_)),
    stop_handler(std::move(stop_handler_)),
    strand(ctx.get_io_service()), socket(ctx.get_io_service()),
    resolver(ctx.get_io_service()), next_xid(0) {

}

void rpc_connection::start(std::string hostname, uint16_t port) {
    remote_hostname = std::move(hostname);
    remote_port = port;

    LOG(INFO) << ctx.get_local_node_id() << ":" << get_conn_id()
              << " Connecting to: "
              << remote_hostname << ":" << remote_port;

    auto self = shared_from_this();
    auto handle_connect =
        [self, this](const error_code& ec,
                     ba::ip::tcp::resolver::iterator it) {
        if (ec) {
            if (ec != ba::error::operation_aborted) {
                LOG(WARNING) << ctx.get_local_node_id() << ":" << get_conn_id()
                             << " Failed to connect " << get_conn_id() << " to "
                             << remote_hostname << ":" << remote_port
                             << ": " << ec.message();
                if (it == ba::ip::tcp::resolver::iterator()) {
                    LOG(ERROR) << ctx.get_local_node_id() << ":" << get_conn_id()
                               << " Fatal connection error";
                    stop();
                }
            }
        } else {
            LOG(INFO) << ctx.get_local_node_id() << ":" << get_conn_id()
                      << " Connected to "
                      << remote_hostname << ":" << remote_port;
            handler->handle_connect(*this);
            read_message();
        }
    };
    auto handle_resolve =
        [self, this, handle_connect](const error_code& ec,
                                     ba::ip::tcp::resolver::iterator it) {
        if (ec) {
            if (ec != ba::error::operation_aborted) {
                LOG(ERROR) << ctx.get_local_node_id() << ":" << get_conn_id()
                           << " Failed to resolve " << remote_hostname
                           << ":" << remote_port
                           << ": " << ec.message();
                stop();
            }
            return;
        }

        ba::async_connect(socket, it, handle_connect);
    };

    auto start_connect = [self, this, handle_connect, handle_resolve]() {
        boost::system::error_code ec;
        address addr = address::from_string(remote_hostname, ec);
        if (!ec) {
            ba::ip::tcp::endpoint endpoint(addr, remote_port);
            auto wrap_connect = [self, this, handle_connect](const error_code& ec) {
                handle_connect(ec, ba::ip::tcp::resolver::iterator());
            };
            socket.async_connect(endpoint, strand.wrap(wrap_connect));
        } else {
            resolver.async_resolve({remote_hostname, std::to_string(remote_port),
                                    ba::ip::tcp::resolver::query::numeric_service},
                                   strand.wrap(handle_resolve));
        }
    };

    // XXX TODO start timeout timer
    strand.dispatch(start_connect);
}

void rpc_connection::start() {
    // XXX TODO start timeout timer
    auto self = shared_from_this();
    strand.dispatch([self, this]() {
            LOG(INFO) << ctx.get_local_node_id() << ":" << get_conn_id()
                      << " New remote connection from "
                      << socket.remote_endpoint();

            handler->handle_connect(*this);
            read_message();
        });
}

void rpc_connection::stop() {
    // XXX TODO stop timeout timer
    auto self = shared_from_this();
    strand.dispatch([self, this]() {
            LOG(INFO) << ctx.get_local_node_id() << ":" << get_conn_id()
                      << " Closing connection";
            socket.close();
            stop_handler(self);
        });
}

void rpc_connection::send_message(message::rpc_message& m) {
    if (!m.has_xid()) m.set_xid(next_xid++);

    auto writebuf = make_shared<vector<uint8_t>>();
    uint32_t msg_len = m.ByteSize();
    writebuf->resize(4 + msg_len);
    *((uint32_t*)&(*writebuf)[0]) = htonl(msg_len);
    m.SerializeToArray(&(*writebuf)[4], msg_len);

    auto self = shared_from_this();
    auto do_write = [self, this, writebuf]() {
        ba::async_write(socket, ba::buffer(*writebuf),
                        [self, this, writebuf](const error_code& ec,
                                               size_t bytes_transferred) {
                            if (!ec) last_write = steady_clock::now();
                        });
    };
    strand.dispatch(do_write);
}

void rpc_connection::send_error_message(uint64_t xid,
                                        message::method method,
                                        message::status status_code,
                                        const std::string& status_message) {
    message::rpc_message message;
    message.set_xid(xid);
    message.set_method(method);

    message::rpc_rep* rep = message.mutable_rep();
    rep->set_status_code(status_code);
    rep->set_status_message(status_message);

    send_message(message);
}

void rpc_connection::read_message() {
    auto self = shared_from_this();
    buffer.resize(4);
    auto handle_size = [self, this](const error_code& ec, size_t len) {
        if (ec) {
            if (ec != ba::error::operation_aborted) {
                LOG(ERROR) << ctx.get_local_node_id() << ":" << get_conn_id()
                          << " Could not read from socket: "
                          << ec.message();
                stop();
            }
            return;
        }

        size_t msg_len = ntohl(*(uint32_t*)(&buffer[0]));
        if (len > 1024*1024*64) {
            LOG(ERROR) << ctx.get_local_node_id() << ":" << get_conn_id()
                       << " Invalid message length: " << msg_len;
            stop();
            return;
        }
        buffer.resize(msg_len);

        auto handle_body = [self, this](const error_code& ec, size_t len) {
            if (ec) {
                if (ec != ba::error::operation_aborted) {
                    LOG(ERROR) << ctx.get_local_node_id() << ":" << get_conn_id()
                               << " Could not read from socket: "
                               << ec.message();
                    stop();
                }
                return;
            }

            message::rpc_message m;
            google::protobuf::io::ArrayInputStream s(&buffer[0], buffer.size());
            m.ParseFromZeroCopyStream(&s);
            handler->handle_message(*this, m);
            last_read = steady_clock::now();
            read_message();
        };

        ba::async_read(socket, ba::buffer(buffer), strand.wrap(handle_body));
    };
    ba::async_read(socket, ba::buffer(buffer), strand.wrap(handle_size));
}

} /* namespace internal */
} /* namespace throng */
