# User Identification Manager

D-Bus service for identifying users from external sources in an automotive setting.

# Dependencies

- gdbus-codgen-glibmm
- glibmm (2.56)
- googletest (1.8.1, for tests, optional)

# Building

[Meson](https://mesonbuild.com/) is used for building. To build simply run:

```shell
meson build
ninja -C build
```

# Running Tests

Tests can be run with:

```shell
meson test -C build
```

"No tests defined." is printed if the required version of googletest could not be found.

# Configuration

The daemon reads configuration from `/etc/user-identification-manager.conf`. A default configuration
will be used if the file does not exist. Another path can be used by passing the `-c`/`--config`
argument to the daemon.

Content that corresponds to default configuration with comments explaining the variables:

```
[sources]
# Comma separated list of sources to enable. Empty means all.
enable=
```

# Command Line Interface

`uimcli` is a command line tool that can be used to debug and monitor the User Identification
Manager daemon. Help summary:

```
$ uimcli -h
Usage:
  uimcli [OPTION…]

Help Options:
  -h, --help                 Show help options

Application Options:
  --version                  Print version and exit
  -i, --identified-users     Print recently identified users
  -s, --sources              Print enabled and disabled identification sources
  -m, --monitor              Monitor for users to be identified
```

# License and Copyright

Copyright © 2019 Luxoft Sweden AB

The source code in this repository is subject to the terms of the MPL-2.0 license, please see
included "LICENSE" file for details. License information for any other files is either explicitly
stated or defaults to the MPL-2.0 license.
