#!/usr/bin/bash

set -x

mkdir -p build &&
cd build &&
conan install .. --profile ../conanprofile.toml --build missing &&
cmake .. -DCMAKE_BUILD_TYPE=Debug -G"Ninja" -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=YES &&
ninja &&
cp -f compile_commands.json ../
