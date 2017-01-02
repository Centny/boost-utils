#!/bin/bash
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp #**/*.hpp **/*.cc **/*.c
gyp --depth=. --build=Default

if [ "$1" = "test" ];then
    ./build/Default/boost-utils-test
fi