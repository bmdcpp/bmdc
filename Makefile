all:
	mkdir -p ${HOME}/test2/
	scons PREFIX=${HOME}/test2/ release=1
Release:
	mkdir -p ${HOME}/test2/
	scons PREFIX=${HOME}/test2/ release=1
	scons install
Debug:
	mkdir -p ${HOME}/test2/
	scons PREFIX=${HOME}/test2/ debug=1
	scons install
cleanrelease:
	scons -c
cleandebug:
	scons debug=1 -c
