#!/bin/sh
export PATH="${PATH}";
echo $PATH;
export CC="clang";
echo $CC;
export CXX="clang++";
echo $CXX;
mkdir -p $HOME/clang-bmdc/
scons --debug=includes PREFIX=$HOME/clang-bmdc/ debug=1 -j3
#scons install
