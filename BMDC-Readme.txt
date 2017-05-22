Introduction:
-------------
BMDC++ - DC++ client based on FreeDC++ with Ignore Users,... 
see BMDC-Changelog.txt for detailed info.

Dependencies:
-------------
- scons >= 0.96
- pkg-config
- g++ >= 4.7
- glib >= 2.32
- gtk+-3.0 >= 3.6
- pthread
- zlib
- libbz2
- libssl
- libGeoIP >= 1.4.7

--optional--

- libXss (Note: for idle detection)
- libtar (Note: For Backup/Restore options )
- libnotify >= 0.4.1 ( Note: For popups notifications )
- libappindicator3 ( Note: For appindicators)
- xattr ( Note: for storing hashes beside files to not always re-hash )
- Runtime-deps is also glib-networking ( or similar) for open URI
- also python2-dbus and qdbus for Media Spam (/kaff and /vlc)

Compiling:
----------
$ cd /path/to/bmdc-source
$ scons PREFIX=/path/to/install/

Installing:
-----------
# scons install

Running:
--------
$ bmdc
or
$ /path/to/install/bin/bmdc

Uninstalling:
-------------
To uninstall BMDC++, simply delete the directory you installed it in. To uninstall using scons, run:

$ scons -c

or if you used scons to install into the file system then use the same scons command that you used to install and add the option -c:

# scons -c install

License:
--------
GNU GPL Version 2 or later
See License.txt for details.

Contact:
--------
Website: 
	https://launchpad.net/bmdc++
	https://sourceforge.net/projects/freedcppmv/
