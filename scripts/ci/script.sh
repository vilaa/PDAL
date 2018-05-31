#!/bin/sh -e
# Builds and tests PDAL

gcc --version
g++ --version

mkdir -p _build || exit 1
cd _build || exit 1

GEOTIFF_INCLUDE_DIR=$CONDA_PREFIX/include \
cmake .. \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_MAKE_PROGRAM=make \
    -DBUILD_PLUGIN_PYTHON=ON \
    -DBUILD_PLUGIN_CPD=OFF \
    -DBUILD_PLUGIN_GREYHOUND=ON \
    -DBUILD_PLUGIN_HEXBIN=ON \
    -DBUILD_PLUGIN_NITF=ON \
    -DBUILD_PLUGIN_ICEBRIDGE=ON \
    -DBUILD_PLUGIN_PGPOINTCLOUD=ON \
    -DBUILD_PGPOINTCLOUD_TESTS=OFF \
    -DBUILD_PLUGIN_SQLITE=ON \
    -DWITH_LASZIP=ON \
    -DWITH_LAZPERF=ON \
    -DWITH_TESTS=ON

make -j2
LD_LIBRARY_PATH=./lib
ctest -V
make install
