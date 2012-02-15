all:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ release=1 libgnome=0 libnotify=0 -j2
Release:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ release=1 libgnome=0 libnotify=0 -j2
	scons install
Debug:
	mkdir -p ${HOME}/BMDC/
	scons PREFIX=${HOME}/BMDC/ debug=1 libgnome=0 -j2
	scons install
cleanrelease:
	scons -c
cleandebug:
	scons debug=1 -c
