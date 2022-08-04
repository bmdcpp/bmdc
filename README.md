# BMDC++ &ndash; file sharing program using Direct Connect protocols

Website: https://bmdcpp.github.io/<br/>
Sources: https://github.com/bmdcpp/bmdc

## Description

BMDC++ is a cross-platform program that uses the [Direct Connect](https://en.wikipedia.org/wiki/Direct_Connect_\(protocol\)) and [Advanced Direct Connect](https://en.wikipedia.org/wiki/Advanced_Direct_Connect) protocols. It is compatible with DC++, AirDC++, EiskaltDC++, FlylinkDC++ and other [DC clients](https://en.wikipedia.org/wiki/Comparison_of_ADC_software#Client_software). BMDC++ also interoperates with all common DC hub software.

BMDC++ client was based on FreeDC++ code base. Program was ported to GTK+ 3.x, Ignore Users and other useful features were added. See [BMDC-Changelog.txt](./BMDC-Changelog.txt) for detailed info.

## License

GNU GPL v2.0 or later. See [License.txt](./License.txt) for details.

## Dependencies

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

### Optional dependencies

- libXss (Note: for idle detection)
- libtar (Note: For Backup/Restore options )
- libnotify >= 0.4.1 ( Note: For popups notifications )
- libappindicator3 ( Note: For appindicators)
- xattr ( Note: for storing hashes beside files to not always re-hash )
- Runtime-deps is also glib-networking ( or similar) for open URI
- also python2-dbus and qdbus for Media Spam (/kaff and /vlc)

## Compiling and installing

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

To uninstall BMDC++, simply delete the directory you installed it in.

To uninstall using scons, run:

```
$ scons -c
```

or if you used scons to install into the file system then use the same scons command that you used to install and add the option -c:

```
# scons -c install
```

## Contact
[@Mank16](https://www.github.com/Mank16)
[@bmdcpp](https://www.github.com/@bmdcpp)


