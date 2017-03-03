all:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ release=1
	scons install
Release:
	mkdir -p ${HOME}/BMDC-release/
	scons PREFIX=${HOME}/BMDC-release/ release=1 libxattr=0 newSettings=0
	scons install
	chmod +x ${HOME}/BMDC-release/bin/bmdc
Debug:
	mkdir -p ${HOME}/debug
	scons PREFIX=${HOME}/debug/ debug=1 profile=0 libappindicator=1 libxattr=0 newSettings=1
	#scons install
cleanrelease:
	scons -c
cleandebug:
	scons debug=1 -c
cmake-build:
	mkdir -p cmake-build;cd cmake-build;
	cmake -DENABLE_CANBERRA=OFF -DCMAKE_PREFIX_PATH=/usr/lib64 -DUSE_MINIUPNP=ON -DCMAKE_INSTALL_PREFIX=/home/mank1/bmdc-debug ..
	make install

