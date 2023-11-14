#!/bin/bash
basepath=$(cd `dirname $0`; pwd)
dir=$basepath/BuildDebug
mkdir -p "$dir"
cd "$dir"
cmake "-DCMAKE_BUILD_TYPE=Debug" ..
make -j8
