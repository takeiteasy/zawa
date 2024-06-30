#!/bin/sh

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
