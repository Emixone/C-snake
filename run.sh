#!/bin/sh

PROJECT=snake

mkdir -p build
cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..
cd ..
cmake --build ./build --config Debug --target $PROJECT -j8 && \
        ./build/$PROJECT

