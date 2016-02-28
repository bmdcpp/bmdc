all:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ release=1
	scons install
Release:
	mkdir -p /media/CLS/BMDC-release/
	scons PREFIX=/media/CLS/BMDC-release/ release=1 libxattr=1 newSettings=0
	scons install
	chmod +x /media/CLS/BMDC-release/bin/bmdc
Debug:
	mkdir -p ${HOME}/debug
	scons PREFIX=${HOME}/debug/ debug=1 profile=0 libappindicator=1 libxattr=1 newSettings=0
	scons install
cleanrelease:
	scons -c
cleandebug:
	scons debug=1 -c
