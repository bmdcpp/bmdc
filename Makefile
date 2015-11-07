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
	mkdir -p /media/CLS/BMDC/
	scons PREFIX=/media/CLS/BMDC/ debug=1 profile=0 libappindicator=0 libxattr=1 newSettings=1 -j2
	scons install
cleanrelease:
	scons -c
cleandebug:
	scons debug=1 -c
