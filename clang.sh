#!/bin/sh
export PATH="${PATH}";
echo $PATH;
export CC="clang";
echo $CC;
export CXX="clang++";
echo $CXX;
#export CCFLAGS="${CCFLAGS} -std=c++11 -pthread";
#echo $CCFLAGS;
#export CPPFLAGS="${CPPFLAGS} -pthread";
#echo $CPPFLAGS;
#export CXXFLAGS="${CPPFLAGS}";
#echo $CXXFLAGS;
#-stdlib=libstdc++
#export CFLAGS="${CFLAGS}  -std=c++11 -pthread";
#echo $CFLAGS;
#export LINKFLAGS="${LINKFLAGS} -pthread";
#echo $LINKFLAGS;
mkdir -p $HOME/clang-bmdc/
scons --debug=includes PREFIX=$HOME/clang-bmdc/ debug=1 
scons install
