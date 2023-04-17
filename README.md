[![GPL Licence](https://badges.frapsoft.com/os/gpl/gpl.png?v=103)](https://opensource.org/licenses/GPL-3.0/) 
[![CodeQL](https://github.com/bmdcpp/bmdc/actions/workflows/codeql.yml/badge.svg?branch=main)](https://github.com/bmdcpp/bmdc/actions/workflows/codeql.yml)
![Maintenance](https://img.shields.io/maintenance/yes/2023)

# BMDC &ndash; file sharing using DC and ADC protocols

## Introduction

BMDC &ndash; DC++ client based on FreeDC++ with Ignore Users and other changes.<br/>
See [BMDC-Changelog.txt](https://github.com/bmdcpp/bmdc/blob/master/BMDC-Changelog.txt) for detailed info.

## License

GNU GPL v2.0 or later. See [License.txt](https://github.com/bmdcpp/bmdc/blob/master/License.txt) for details.

## Links

- Website: https://bmdcpp.github.io/bmdc
- Sources: https://github.com/bmdcpp/bmdc

## Dependencies

- scons >= 0.96
- pkg-config
- g++ >= 4.7
- glib >= 2.32
- gtk+ >= 4.0
- pthread
- zlib
- libbz2
- libssl
- libmaxminddb
- gettext
- pcre

### Optional dependencies

- libtar (Note: For Backup/Restore options )
- libnotify >= 0.4.1 ( Note: For popups notifications )
- xattr ( Note: for storing hashes beside files to not always re-hash )
- Runtime-deps is also glib-networking ( or similar) for open URI
- also python2-dbus and qdbus for Media Spam (/kaff and /vlc)


## Compiling and installing
=======

### Compiling

```
$ cd /path/to/bmdc-source
$ scons PREFIX=/path/to/install/
```

### Installing

```
# scons install
```

## Running
```
$ bmdc
```

or

```
$ /path/to/install/bin/bmdc
```

## Uninstalling

To uninstall BMDC, simply delete the directory you installed it in.

To uninstall using scons, run:

```
$ scons -c
```

or if you used scons to install into the file system then use the same scons command that you used to install and add the option -c:

```
# scons -c install
```

=======

## License

GNU GPL Version 2 or later
See License.txt for details.

## Contact
[@bmdcpp](https://www.github.com/@bmdcpp)

