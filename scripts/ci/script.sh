#!/bin/sh -e
# Builds and tests PDAL

gcc --version
g++ --version

mkdir -p _build || exit 1
cd _build || exit 1

cmake .. \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_MAKE_PROGRAM=make \
    -DBUILD_PLUGIN_PYTHON=ON \
    -DBUILD_PLUGIN_CPD=OFF \
    -DBUILD_PLUGIN_GREYHOUND=OFF \
    -DBUILD_PLUGIN_HEXBIN=OFF \
    -DBUILD_PLUGIN_NITF=OFF \
    -DBUILD_PLUGIN_ICEBRIDGE=OFF \
    -DBUILD_PLUGIN_PGPOINTCLOUD=OFF \
    -DBUILD_PGPOINTCLOUD_TESTS=OFF \
    -DBUILD_PLUGIN_SQLITE=OFF \
    -DWITH_LASZIP=ON \
    -DWITH_LAZPERF=ON \
    -DWITH_TESTS=ON \
    -DGDAL_INCLUDE_DIR=$CONDA_PREFIX/include \
    -DGDAL_LIBRARY=$CONDA_PREFIX/lib/libgdal.so \
    -DGEOTIFF_INCLUDE_DIR=$CONDA_PREFIX/include \
    -DGEOTIFF_LIBRARY=$CONDA_PREFIX/lib/libgeotiff.so \
    -DHEXER_INCLUDE_DIR=$CONDA_PREFIX/include \
    -DHEXER_LIBRARY=$CONDA_PREFIX/lib/libhexer.so \
    -DJSONCPP_INCLUDE_DIR=$CONDA_PREFIX/include \
    -DJSONCPP_LIBRARY=$CONDA_PREFIX/lib/libjsoncpp.so \
    -DNITRO_INCLUDE_DIR=$CONDA_PREFIX/include \
    -DNITRO_C_LIBRARY=$CONDA_PREFIX/lib/libnitf-c.so \
    -DNITRO_CPP_LIBRARY=$CONDA_PREFIX/lib/libnitf-cpp.so

make -j2
LD_LIBRARY_PATH=./lib
ctest -V
make install
