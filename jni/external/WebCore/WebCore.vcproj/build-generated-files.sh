#!/usr/bin/bash

# Copyright (C) 2007 Apple Inc.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer. 
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution. 
# 3.  Neither the name of Apple puter, Inc. ("Apple") nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

NUMCPUS=`../../WebKitTools/Scripts/num-cpus`

XSRCROOT="`pwd`/.."
XSRCROOT=`realpath "$XSRCROOT"`
# Do a little dance to get the path into 8.3 form to make it safe for gnu make
# http://bugzilla.opendarwin.org/show_bug.cgi?id=8173
XSRCROOT=`cygpath -m -s "$XSRCROOT"`
XSRCROOT=`cygpath -u "$XSRCROOT"`
export XSRCROOT
export SOURCE_ROOT=$XSRCROOT

XDSTROOT="$1"
export XDSTROOT
# Do a little dance to get the path into 8.3 form to make it safe for gnu make
# http://bugzilla.opendarwin.org/show_bug.cgi?id=8173
XDSTROOT=`cygpath -m -s "$XDSTROOT"`
XDSTROOT=`cygpath -u "$XDSTROOT"`
export XDSTROOT

SDKROOT="$2"
export SDKROOT
# Do a little dance to get the path into 8.3 form to make it safe for gnu make
# http://bugzilla.opendarwin.org/show_bug.cgi?id=8173
SDKROOT=`cygpath -m -s "$SDKROOT"`
SDKROOT=`cygpath -u "$SDKROOT"`
export SDKROOT

export BUILT_PRODUCTS_DIR="$XDSTROOT/obj/WebCore"

if [ -e "$XDSTROOT/include/JavaScriptCore/create_hash_table" ]; then
    export CREATE_HASH_TABLE="$XDSTROOT/include/JavaScriptCore/create_hash_table"
elif [ -e "$XDSTROOT/include/private/JavaScriptCore/create_hash_table" ]; then
    export CREATE_HASH_TABLE="$XDSTROOT/include/private/JavaScriptCore/create_hash_table"
elif [ -e "$SDKROOT/include/JavaScriptCore/create_hash_table" ]; then
    export CREATE_HASH_TABLE="$SDKROOT/include/JavaScriptCore/create_hash_table"
elif [ -e "$SDKROOT/include/private/JavaScriptCore/create_hash_table" ]; then
    export CREATE_HASH_TABLE="$SDKROOT/include/private/JavaScriptCore/create_hash_table"
fi

echo ${CREATE_HASH_TABLE}

mkdir -p "${BUILT_PRODUCTS_DIR}/DerivedSources"
cd "${BUILT_PRODUCTS_DIR}/DerivedSources"

export WebCore="${XSRCROOT}"
export FEATURE_DEFINES="ENABLE_CHANNEL_MESSAGING ENABLE_DATABASE ENABLE_DATAGRID ENABLE_DOM_STORAGE ENABLE_ICONDATABASE ENABLE_JAVASCRIPT_DEBUGGER ENABLE_OFFLINE_WEB_APPLICATIONS ENABLE_RUBY ENABLE_SVG ENABLE_SVG_ANIMATION ENABLE_SVG_AS_IMAGE ENABLE_SVG_FONTS ENABLE_SVG_FOREIGN_OBJECT ENABLE_SVG_USE ENABLE_VIDEO ENABLE_WEB_SOCKETS ENABLE_WORKERS ENABLE_XPATH ENABLE_XSLT"
make -f "$WebCore/DerivedSources.make" -j ${NUMCPUS} || exit 1
