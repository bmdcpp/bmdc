#!/bin/bash
export CC="clang -std=c++11";
export CXX="clang++ -std=c++11";
export CCFLAGS="${CCFLAGS} -std=c++11 -pthread -stdlib=libstdc++";
export CPPFLAGS="${CPPFLAGS}-pthread -stdlib=libstdc++";
export CFLAGS="${CFLAGS} -stdlib=libstdc++ -std=c++11 -pthread";
export LINKFLAGS="{LINKFLAGS} -pthread";
mkdir -p $HOME/clang-bmdc/
scons PREFIX=$HOME/clang-bmdc/ debug=1
scons install
