#!/bin/bash
export CC="clang -pthread -w";
export CXX="clang++ -pthread -w";
mkdir -p $HOME/clang-bmdc/
scons PREFIX=$HOME/clang-bmdc/ debug=1
scons install
