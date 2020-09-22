# Installing Boost 1.73.0 which is the only version that supports
# Beast and websockets
wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz

tar zxvf boost_1_73_0.tar.gz

cd boost_1_73_0 && ./bootstrap.sh --with-libraries=all --with-toolset=gcc && ./b2

# Installing jsoncpp
# Make sure to use -ljsoncpp when compiling
sudo apt-get install libjsoncpp-dev

# Cleanup
rm boost_1_73_0.tar.gz
