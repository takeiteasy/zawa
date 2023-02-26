#!/bin/sh

download() {
    wget "$1" -O "$2"deps/$(basename "$1")
}

# Download latest header-only dependencies
download "https://raw.githubusercontent.com/floooh/sokol/master/sokol_gfx.h"
download "https://raw.githubusercontent.com/floooh/sokol/master/sokol_glue.h"
download "https://raw.githubusercontent.com/floooh/sokol/master/sokol_app.h"
download "https://raw.githubusercontent.com/floooh/sokol/master/sokol_audio.h"
download "https://raw.githubusercontent.com/thisistherk/fast_obj/master/fast_obj.h" "tools/"
download "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "tools/"

cd deps/
wget https://bitbucket.org/odedevs/ode/downloads/ode-0.16.3.tar.gz
tar -xf ode-0.16.3.tar.gz
mv ode-0.16.3/ ode/
cd ode/
patch -p1 < ../../tools/mass.patch
cd build/
cmake ..
make
cd ../../
rm ode-0.16.3.tar.gz
cd ../

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
