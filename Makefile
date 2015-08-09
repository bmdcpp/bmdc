all:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ release=1
	scons install
Release:
	mkdir -p ${HOME}/BMDC-release/
	scons PREFIX=${HOME}/BMDC-release/ release=1 libxattr=1 -j9
	scons install
	chmod +x ${HOME}/BMDC-release/bin/bmdc
Debug:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ debug=1 profile=0 libappindicator=0 libxattr=1 -j9
	scons install
cleanrelease:
	scons -c
cleandebug:
	scons debug=1 -c
