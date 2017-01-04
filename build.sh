#!/bin/bash
set -e
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp #**/*.hpp **/*.cc **/*.c
gyp --depth=. --build=Default
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
if [ "$1" = "test" ];then
    cd ./build/Default/
    ./boost.utils.test
    cd ../../
fi

if [ "$1" = "run" ];then
    cd ./build/Default/
    ./boost.utils.test.console $2
    cd ../../
fi