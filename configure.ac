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

#
# Process this file with autoconf to produce a configure script
#
# If you just want to start a build from source control, run
# autogen.sh first.
#

# ---------------------------------------------------------------
# Initialization

AC_INIT([libthrong], [1.0.0])

# For more information on libtool versioning, see:
# https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#
# The version string is [current:revision:age] or [c:r:a].  Here are a
# set of rules to help you update your library version information:
#  1. Start with version information of 0:0:0 for each libtool
#     library.
#  2. Update the version information only immediately before a public
#     release of your software. More frequent updates are unnecessary,
#     and only guarantee that the current interface number gets larger
#     faster.
#  3. If the library source code has changed at all since the last
#     update, then increment revision (c:r:a becomes c:r+1:a).
#  4. If any interfaces have been added, removed, or changed since the
#     last update, increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release,
#     then increment age.
#  6. If any interfaces have been removed since the last public
#     release, then set age to 0.
AC_SUBST(VERSION_INFO, [0:0:0])

# initialize automake and libtool
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_CONFIG_HEADER(config.h)
LT_INIT()
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

m4_include([m4/ax_cxx_compile_stdcxx_11.m4])
m4_include([m4/ax_pkg_check_modules.m4])
m4_include([m4/ax_boost_base.m4])
m4_include([m4/ax_boost_unit_test_framework.m4])
m4_include([m4/ax_boost_program_options.m4])
m4_include([m4/ax_boost_system.m4])
m4_include([m4/ax_boost_filesystem.m4])
m4_include([m4/ax_boost_iostreams.m4])
m4_include([m4/ax_boost_asio.m4])
m4_include([m4/ax_boost_random.m4])

# ---------------------------------------------------------------
# Configuration options
# Modify the release/build version

AC_ARG_WITH(buildversion,
            AC_HELP_STRING([--with-buildversion],
                           [Version number of build]),
            [bversion=${withval}],
            [bversion='private'])
AC_SUBST(BVERSION, [${bversion}])

# Allow --disable-assert option for final builds
AC_HEADER_ASSERT

# ---------------------------------------------------------------
# Environment introspection

# check for compiler
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_AS
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11(noext,mandatory)

# ---------------------------------------------------------------
# Dependency checks

# Checks for header files
AC_STDC_HEADERS

# Boost libraries
AX_BOOST_BASE([1.53.0], [], AC_MSG_ERROR([Boost is required]))

AX_BOOST_UNIT_TEST_FRAMEWORK
AS_IF([test "x$ax_cv_boost_unit_test_framework" != "xyes"],
      [AC_MSG_ERROR([Boost::Test library is required.])])
AX_BOOST_RANDOM
AS_IF([test "x$ax_cv_boost_random" != "xyes"],
      [AC_MSG_ERROR([Boost::Random library is required.])])
AX_BOOST_SYSTEM
AS_IF([test "x$ax_cv_boost_system" != "xyes"],
      [AC_MSG_ERROR([Boost::System library is required.])])
AX_BOOST_FILESYSTEM
AS_IF([test "x$ax_cv_boost_filesystem" != "xyes"],
      [AC_MSG_ERROR([Boost::Filesystem library is required.])])
AX_BOOST_IOSTREAMS
AS_IF([test "x$ax_cv_boost_iostreams" != "xyes"],
      [AC_MSG_ERROR([Boost::IOStreams library is required.])])
AX_BOOST_ASIO
AS_IF([test "x$ax_cv_boost_asio" != "xyes"],
      [AC_MSG_ERROR([Boost::ASIO library is required.])])

# Package config dependencies
AX_PKG_CHECK_MODULES([protobuf], [], [protobuf >= 2.5.0])

# LevelDB
AC_CACHE_CHECK(whether the leveldb library is available,
    ax_cv_leveldb,
    [AC_LANG_PUSH([C++])
    CXXFLAGS_SAVE=$CXXFLAGS
    
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    	[[@%:@include <leveldb/options.h>]],
    	[[leveldb::Options o;]])],
    	ax_cv_leveldb=yes, ax_cv_leveldb=no)
    	CXXFLAGS=$CXXFLAGS_SAVE
    AC_LANG_POP([C++])
])
AS_IF([test "x$ax_cv_leveldb" != "xyes"],
      [AC_MSG_ERROR([Leveldb library is required.])])
AC_CHECK_LIB(leveldb, leveldb_options_create,
             [leveldb_LIBS="-lleveldb"; AC_SUBST(leveldb_LIBS) link_leveldb="yes"],
	     [link_leveldb="no"])
AS_IF([test "x$link_leveldb" != "xyes"],
      [AC_MSG_ERROR([Could not link -lleveldb.])])
						
# Protocol buffers compiler
AC_CHECK_PROG([PROTOC], [protoc], [protoc])
AS_IF([test "x${PROTOC}" == "x"],
      [AC_MSG_ERROR([Protocol buffer compiler "protoc" is required.])])

# check for doxygen
AC_CHECK_PROGS([DOXYGEN], [doxygen], [doxygen])
AS_IF([test "x${DOXYGEN}" == "x"],
      [AC_MSG_ERROR([Doxygen documentation builder "doxygen" is required.])])

# ---------------------------------------------------------------
# Output

AC_CONFIG_FILES([\
	Makefile \
	libthrong.pc \
	doc/Doxyfile \
	doc/Doxyfile-internal])
AC_OUTPUT

AC_MSG_NOTICE([
======================================================================
Configuration complete

You may now compile the software by running 'make'
======================================================================])

