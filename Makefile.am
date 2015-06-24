#
# throng: Framework for large-scale policy-driven orchestration
# Copyright (c) 2015 Cisco Systems, Inc. and others.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#
###########
#
# Process this file with automake to produce a Makefile.in

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(BOOST_CPPFLAGS) \
        -I$(top_srcdir)/src/include -I$(top_srcdir)/include \
	-I$(builddir)/proto
AM_LDFLAGS = $(BOOST_LDFLAGS)

throng_includedir = $(includedir)/throng
throng_include_HEADERS = \
	include/throng/vector_clock.h \
	include/throng/versioned.h \
	include/throng/store.h \
	include/throng/store_client.h \
	include/throng/serializer.h \
	include/throng/ctx.h

protobuf_headers = \
	proto/throng_messages.pb.h
protobuf_sources = \
	proto/throng_messages.pb.cc
BUILT_SOURCES = $(protobuf_sources) $(protobuf_headers)
MOSTLYCLEANFILES = $(BUILT_SOURCES)

noinst_HEADERS = \
	src/include/storage_engine.h \
	src/include/in_memory_storage_engine.h\
	$(protobuf_headers)

lib_LTLIBRARIES = libthrong.la
libthrong_la_LDFLAGS = -version-info ${VERSION_INFO}
nodist_libthrong_la_SOURCES = $(protobuf_sources)
libthrong_la_CXXFLAGS = $(protobuf_CFLAGS) $(leveldb_CFLAGS)
dependency_libs = $(protobuf_LIBS) $(leveldb_LIBS) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_ASIO_LIB) \
	$(BOOST_RANDOM_LIB)
libthrong_la_SOURCES = \
	src/ctx.cpp \
	src/vector_clock.cpp \
	src/store_registry.cpp \
	src/in_memory_storage_engine.cpp

libthrong_la_LIBADD = $(dependency_libs)
TESTS = throng_test
noinst_PROGRAMS = $(TESTS)

throng_test_CXXFLAGS = \
	-I$(top_srcdir)/test/include
throng_test_LDADD = \
	libthrong.la $(dependency_libs) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LIB)
throng_test_SOURCES = \
	test/main.cpp \
	test/ctx_test.cpp \
	test/vector_clock_test.cpp \
	test/versioned_test.cpp \
	test/in_memory_storage_engine_test.cpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libthrong.pc

docs = README.md
EXTRA_DIST = $(docs) \
	proto/throng_messages.proto


noinst_DATA = doc/html
doc/html: $(docs) $(throng_include_HEADERS) doc/Doxyfile
	cd doc && ${DOXYGEN} Doxyfile
doc: doc/html
doc/html-internal: clean-doc-internal doc/Doxyfile-internal
	cd doc && ${DOXYGEN} Doxyfile-internal
doc-internal: doc/html-internal

install-data-local: doc
	@$(NORMAL_INSTALL)
	test -z "${DESTDIR}/${docdir}/html" || $(mkdir_p) "${DESTDIR}/${docdir}/html"
	for i in `ls $(srcdir)/doc/html`; do \
		$(INSTALL) -m 0644 $(srcdir)/doc/html/$$i "${DESTDIR}/${docdir}/html"; \
	done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -rf "${DESTDIR}/${docdir}/html"
	rm -rf "${DESTDIR}/${includedir}/throng"
clean-doc:
	rm -rf doc/html doc/latex doc/doxygen_sqlite3.db
clean-doc-internal:
	rm -rf doc/html-internal doc/latex-internal
clean-local: clean-doc
	rm -f *.rpm *.deb

%.pb.h %.pb.cc: %.proto
	mkdir -p $(builddir)/proto
	$(PROTOC) --proto_path=$(srcdir)/proto \
		--cpp_out=$(builddir)/proto \
		$<
