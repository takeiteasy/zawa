#!/bin/sh

cd tools/

# Download sokol-shdc-tools binaries
rm -rf bin/ || true
git clone https://github.com/floooh/sokol-tools-bin
mkdir bin/
mv sokol-tools-bin/bin/* bin/
rm -rf sokol-tools-bin

# Make project tools
make

cd ../
