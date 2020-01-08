# User Identification Manager

D-Bus service for identifying users from external sources in an automotive setting.

A D-Bus signal, `UserIdentified`, is emitted when a user is identified. See the
[D-Bus interface](data/com.luxoft.UserIdentificationManager.xml) for details.

# Dependencies

- gdbus-codegen-glibmm
- glibmm (2.56)
- googletest (1.8.1, for tests, optional)
- pcsc-lite (1.8.22, if built with smart card, SCARD, id source)

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

# Identification Sources

What sources to include in the daemon can be changed with [build options](meson_options.txt). For
example, to disable the mass storage device source run `meson build -Dmsd_id_source=false` when
building. When the daemon starts it reads what sources to enable/disable from a
[configuration file](#Configuration). All sources included when building are enabled by default.

## Mass Storage Device Source (MSD)

Reads identification data from a file called `pelux-user-id` in the root of a mass storage device,
e.g. a USB stick. Meant to be used for demonstration and testing purposes. The file must be formated
as follows (replace &lt;X&gt; with actual values):

```
ID <numeric string>
SEAT <hex value between 0x0 and 0xffff, must start with 0x>
```

## Smart Card Source (SCARD)

Extract identification from smart cards. Currently only extracts the UID (unique identifier) from
contactless smarts cards that is then used as a means to identify a user.

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
  -i, --identified-users     Print users identified since start of daemon (max 20)
  -s, --sources              Print enabled and disabled identification sources
  -m, --monitor              Monitor user identification events
```

# License and Copyright

Copyright © 2019 Luxoft Sweden AB

The source code in this repository is subject to the terms of the MPL-2.0 license, please see
included "LICENSE" file for details. License information for any other files is either explicitly
stated or defaults to the MPL-2.0 license.
