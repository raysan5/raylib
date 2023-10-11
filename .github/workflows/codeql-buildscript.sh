#!/usr/bin/env bash

cmake -E make_directory build

sudo apt-get update -qq
sudo apt-get install -y gcc-multilib
sudo apt-get install -y --no-install-recommends libglfw3 libglfw3-dev libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev libxext-dev libxfixes-dev

BUILD_TYPE=Release
cd build
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DPLATFORM=Desktop
cmake --build . --config $BUILD_TYPE
