#! /bin/bash

SHELL_DIR=$(pwd)
BUILD_DIR=$(pwd)/build
CCI_BUILD_DIR=$(pwd)/cci-src/build_x86_64_release
DRIVER_INSTALL_DIR=

echo "SHELL_DIR: $SHELL_DIR"
echo "BUILD_DIR: $BUILD_DIR"
echo "DRIVER_INSTALL_DIR: $DRIVER_INSTALL_DIR"

if [ -d "$CCI_BUILD_DIR" ]; then
    rm -rf "$CCI_BUILD_DIR"
fi

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if [ "$1" = "debug" ]; then
    cmake -DCMAKE_BUILD_TYPE=Debug ..
else
    cmake ..
fi

make

if [ -n "$DRIVER_INSTALL_DIR" ]; then
    cp -rfv $BUILD_DIR/libcascci.* "$DRIVER_INSTALL_DIR"
    cp -rfv $BUILD_DIR/libcubrid*.* "$DRIVER_INSTALL_DIR"
fi
cd "$SHELL_DIR"