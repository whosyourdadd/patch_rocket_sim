#!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE}")" ; pwd -P )
echo $parent_path
OS_TYPE=$(uname -s)
echo "OS type is "$OS_TYPE
cd $parent_path/src/scalloc-1.0.0
sh tools/make_deps.sh
build/gyp/gyp --depth=. scalloc.gyp

if [ "$OS_TYPE" == "Linux" ]; then
        BUILDTYPE=Release make
elif [ "$OS_TYPE" == "Darwin" ]; then
        build/gyp/gyp --depth=. scalloc.gyp --build=Release
        echo "*** LIB is located in $parent_path/src/scalloc-1.0.0/out/Realease/libscalloc.dylib"
else
        echo $OS_TYPE " not support"
fi




