#!/bin/bash
set -e
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp #**/*.hpp **/*.cc **/*.c
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
if [ "$1" = "test" ];then
    export GYP_DEFINES="other_cflags='-fprofile-arcs -ftest-coverage' other_lflags='-fprofile-arcs -ftest-coverage'"
fi
gyp --depth=. --build=Default 

if [ "$1" = "test" ];then
    cd ./build/Default/
    ./boost.utils.test
    cd ../../
    rm -rf report
    mkdir -p report/html
    gcovr --html --html-details -o report/html/coverage.html -r . -v build/boost-utils.build/Default/boost_utils.build/
fi

if [ "$1" = "run" ];then
    cd ./build/Default/
    ./boost.utils.test.console $2
    cd ../../
fi