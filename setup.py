#!/usr/bin/env python3

"""
Copyright (C) 2018 Kaltashkin Eugene <aborche.aborche@gmail.com>
Copyright (C) 2019 Boris Lytochkin <lytboris@gmail.com>
Copyright (C) 2020 Yandex, LLC

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51
Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
"""

""" Persistent Python3 plugin for OpenVPN.  It loads the interpreter
instance and lets it run forever, calling Python functions from the C
code, thus avoiding the overhead of firing up an interpreter complete
with the Python startup library.
"""
import warnings; warnings.simplefilter('default')

import distutils.sysconfig
import os 
import sys

try:
  from setuptools import setup, Extension
except ImportError:
  from distutils.core import setup, Extension

if "Py_DEBUG" not in os.environ:
    Py_DEBUG = []
else:
    Py_DEBUG = [('Py_DEBUG', 1)]

libpython_so = distutils.sysconfig.get_config_var('INSTSONAME')
ext_modules = [
    Extension(
        "openvpn-plugin-python",
        sources=["openvpn-plugin-python.c", "embedpython.c"],
        include_dirs = ["/usr/local/include", "/usr/include/openvpn"],
        library_dirs=["/usr/local/lib", "/usr/lib"],
        define_macros=[('LIBPYTHON_SO', '"' + libpython_so + '"')] + Py_DEBUG,
    ),
]

setup(
    name="openvpn-plugin-python",
    version="0.1.0",
    description="Script OpenVPN with Python 3",
    keywords="authentication,security",
    platforms="Unix",
    long_description=__doc__,
    author="Yandex",
    author_email="noc@yandex.net",
    url="http://yandex.ru/",
    license="GPLv2",
    classifiers=["Topic :: System :: Systems Administration :: "],
    ext_modules=ext_modules,
    packages=['openvpn'],
    package_dir={'openvpn': 'python_module_src/openvpn/'},
)
