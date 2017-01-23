#!/bin/bash
set -e
clang-format -style=file -i -sort-includes **/*.cpp **/*.hpp #**/*.hpp **/*.cc **/*.c
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
if [ "$1" = "test" ];then
    rm -rf build
    export GYP_DEFINES="other_cflags='-fprofile-arcs -ftest-coverage' other_lflags='-fprofile-arcs -ftest-coverage'"
    gyp --depth=. --build=Default  --generator-output=build
else
    gyp --depth=. --build=Default
fi

if [ "$1" = "test" ];then
    cd ./build/Default/
    if [ "$2" = "" ];then
        ./boost.utils.test
    else
        ./boost.utils.test --run=$2
    fi
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