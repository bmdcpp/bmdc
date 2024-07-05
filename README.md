[![GPL Licence](https://badges.frapsoft.com/os/gpl/gpl.png?v=103)](https://opensource.org/licenses/GPL-3.0/) 
[![CodeQL](https://github.com/bmdcpp/bmdc/actions/workflows/codeql.yml/badge.svg?branch=main)](https://github.com/bmdcpp/bmdc/actions/workflows/codeql.yml)
![Maintenance](https://img.shields.io/maintenance/yes/2024)

# BMDC &ndash; file sharing using DC and ADC protocols

## Introduction

BMDC &ndash; DC++ client based on FreeDC++ with Ignore Users and other changes.<br/>
See [BMDC-Changelog.txt](https://github.com/bmdcpp/bmdc/blob/main/BMDC-Changelog.txt) for detailed info.

## License

GNU GPL v3.0 or later. See [LICENSE](https://github.com/bmdcpp/bmdc/blob/main/LICENSE) for details.

## Links

- Website: [https://bmdcpp.github.io](https://bmdcpp.github.io/)
- Sources: [https://github.com/bmdcpp/bmdc](https://github.com/bmdcpp/bmdc)

## Dependencies

- scons >= 0.96
- pkg-config
- g++ >= 5
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

- xattr ( Note: for storing hashes beside files to not always re-hash )
- libtar (Note: For Backup/Restore options )
  
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

GNU GPL Version 3.0 or later
See [LICENSE](https://github.com/bmdcpp/bmdc/blob/main/LICENSE) for details.

## Contact
[@bmdcpp](https://www.github.com/@bmdcpp)

