#!/bin/bash

if [[ "$#" -ne 1 || "$1" == "Release" && "$1" == "Debug" ]]
then 
	echo "Must specify build type: Release or Debug"
	exit 1
fi

# project root
PROJECT_ROOT=`$(pwd)`

# Installing Boost 1.73.0 which is the only version that supports
# Beast and websockets
wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz

tar zxvf boost_1_73_0.tar.gz

cd boost_1_73_0 && ./bootstrap.sh --with-libraries=all --with-toolset=gcc && ./b2

# Installing jsoncpp
# Make sure to use -ljsoncpp when compiling
sudo apt-get install libjsoncpp-dev

# Installing gtest
if [[ $1 = "Debug" ]]
then
	sudo apt-get install libgtest-dev
	cd /usr/src/gtest && sudo cmake CMakeLists.txt && sudo make && sudo cp *.a /usr/lib
fi

# Installing and building webrtc

# Installing depot_tools
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=$(pwd)/depot_tools:$PATH

# Getting the source code for webrtc
cd $PROJECT_ROOT
mkdir webrtc-checkout
cd webrtc-checkout

fetch --nohooks webrtc
cd ./src/build && ./install-build-deps.sh --unsupported
gclient sync 

# going back to 'src'
cd ..
git checkout master 
git new-branch webrtc-build

git checkout master
git pull origin master
gclient sync
git checkout my-branch 
git merge master 

if [[ $1 = "Debug" ]]
then
	gn gen out/Debug && ninja -C out/Debug
else 
	gn gen out/Release --args='is_debug=false' && ninja -C out/Release
fi

# Installing Abseil: A collection of libraries that google's code base depeneds on
cd $PROJECT_ROOT
mkdir abseil
# Cloning Abseil
git clone https://github.com/abseil/abseil-cpp.git

# Building 
cd abseil-cpp
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/../../abseil
cmake --build . --target install
cd ../..

# changing the compiler to clang++
export CXX="/usr/bin/clang++"

# Cleanup
rm -rf depot_tools
rm boost_1_73_0.tar.gz
