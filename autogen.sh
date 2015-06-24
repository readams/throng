#!/bin/sh
#
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

# This autogen script will run the autotools to generate the build
# system.  You should run this script in order to initialize a build
# immediately following a checkout.

for i in m4/*
do
    if [ -L "${i}" ]
    then
        rm "${i}"
    fi
done

autoreconf -fis
