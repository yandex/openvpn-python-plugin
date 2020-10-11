# Description

Persistent Python3 plugin for OpenVPN.  It loads the interpreter instance and lets it run indefinitely, calling Python functions from the C code, thus avoiding the overhead of starting up an interpreter complete with the Python startup library.

Tests show that

```
Full reload: 281
Persistent interpreter: 3
```

for a realistically CPU-intensive script.


# Installation

Either copy the `.so` file and the example script to your relevant destination dirs, or use the FreeBSD port.


# Usage

Add this to your openvpn config (e. g. `/usr/local/etc/openvpn/openvpn_tun1.conf`)

```
plugin /usr/local/lib/openvpn-python-plugin.so /usr/local/etc/openvpn/scripts
```

where the first argument is the plugin `.so` and the second argument is the script dir with the Python scripts of your liking.


# Script examples

You can take the provided example as is and customize it according to your needs.  Be sure to leave the main script file name intact, and from it, feel free to `import` or even `reload()` any other Python module as required.
