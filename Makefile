all:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ release=1
	scons install
Release:
	mkdir -p ${HOME}/BMDC-release/
	scons PREFIX=${HOME}/BMDC-release/ release=1 libxattr=0 newSettings=1 -j3
	scons install
	chmod +x ${HOME}/BMDC-release/bin/bmdc
Debug:
	mkdir -p ${HOME}/debug
	scons PREFIX=${HOME}/debug/ debug=1 profile=0 useStatusIcon=1 libxattr=1 newSettings=1 -j4
cleanrelease:
	scons -c
cleandebug:
	scons debug=1 -c



