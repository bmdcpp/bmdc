#!/bin/sh
export PATH="${PATH}";
echo $PATH;
export CC="x86_64-w64-mingw32-gcc";
echo $CC;
export CXX="x86_64-w64-mingw32-g++";
echo $CXX;
export CPP=${CXX};
export CXXFLAGS="-mthreads -mwindows -fpermissive -DUSE_WIN32_CODE";
export LD_LIBRARY_PATH="/usr/lib/gcc/x86_64-w64-mingw32/:/usr/x86_64-w64-mingw32/lib/:/mingw64/lib/";
export LIBRARY_PATH=${LD_LIBRARY_PATH}  
export PKG_CONFIG_PATH="/usr/x86_64-w64-mingw32/lib/pkgconfig/:/mingw64/lib/pkgconfig/";
export RANLIB="x86_64-w64-mingw32-ranlib";
export AR="x86_64-w64-mingw32-ar";
export AS="x86_64-w64-mingw32-as";
export AR_FOR_TARGET=${AR};
export LDFLAGS='-Wl,-subsystem,windows -s';
## we need override http://www.tarnyko.net/en/?q=node/45
export PKG_CONFIG="/usr/bin/x86_64-w64-mingw32-pkg-config";
mkdir -p $HOME/clang-bmdc/
scons --debug=includes PREFIX=./ debug=1 libxattr=0 libappindicator=0 libtar=0
#x86_64-w64-mingw32-ranlib # sometimes needed /usr/bin/x86_64-w64-mingw32-ranlib build/debug/libdcpp/libdcpp.a and then run scons again

