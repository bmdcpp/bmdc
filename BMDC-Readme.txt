Introduction:
-------------
BMDC++ - DC++ client based on FreeDC++ with Ignore Users,... see
 BMDC-Changelog.txt for detailed info.

Dependencies:
-------------
scons >= 0.96
pkg-config
g++ >= 4.5
gtk+-2.0 >= 2.12
gthread-2.0 >= 2.4
libglade-2.0 >= 2.4
pthread
zlib
libbz2
libssl
libboost
libGeoIP >= 1.4.7
libtar
--------------
optional:
 libgnome 
 libnotify >= 0.4.1
 lua5.1
 libluabind-dev
 liblua5.1-0-dev

Compiling:
----------
$ cd /path/to/bmdc-source
$ scons PREFIX=/path/to/install

Installing:
-----------
# scons install

Running:
--------
$ bmdc

Uninstalling:
-------------
To uninstall BMDC++, simply delete the directory you installed it in. To uninstall using scons, run:

$ scons -c

or if you used scons to install into the file system then use the same scons command that you used to install and add the option -c:

# scons -c install

License:
--------
GNU GPL Version 2
See License.txt for details.

Contact:
--------
Website: 
	https://launchpad.net/bmdc++
	https://sourceforge.net/projects/freedcppmv/
	
