#!/usr/local/bin/python2

'''
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
'''

'''
For detailed description of plug-in functions take a look at
https://github.com/OpenVPN/openvpn/blob/master/include/openvpn-plugin.h.in

Each plug-in method declaration must accept single argument. This argument
holds a set of variables to check as a dict.

Plug-in core checks each method for existence upon startup. Comment out
all methods you do not need, this will prevent plug-in core from registering
for that method thus saving some precious time for packet processing.
'''

import sys, os

OPENVPN_PLUGIN_FUNC_SUCCESS = 0
OPENVPN_PLUGIN_FUNC_ERROR = 1
OPENVPN_PLUGIN_FUNC_DEFERRED = 2

_GLOBALS = {}

def print_dict(env):
    for k, v in env.items():
        print("%s -> %s :: %s" % (k, v, type(v)))
    print("=" * 30)


    """
    Method OPENVPN_PLUGIN_UP called when plug-in started first time from OpenVPN main process
    """
def PLUGIN_UP(env):
    print("OPENVPN_PLUGIN_UP")
    return OPENVPN_PLUGIN_FUNC_SUCCESS

    """
    Method OPENVPN_PLUGIN_DOWN called when OpenVPN main process is shutting down
    """
def PLUGIN_DOWN(env):
    print("OPENVPN_PLUGIN_DOWN")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_ROUTE_UP(env):
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_IPCHANGE(env):
    print("OPENVPN_PLUGIN_IPCHANGE")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_TLS_VERIFY(env):
    print("OPENVPN_PLUGIN_TLS_VERIFY")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_AUTH_USER_PASS_VERIFY(env):
    """
    Method OPENVPN_PLUGIN_AUTH_USER_PASS_VERIFY used for checking username/password pair.
    in OpenVPN debug log password field is not included due security purposes, but exists in argv array
    """
    print("OPENVPN_PLUGIN_AUTH_USER_PASS_VERIFY")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_CLIENT_CONNECT(env):
    print("OPENVPN_PLUGIN_CLIENT_CONNECT\n")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_CLIENT_DISCONNECT(env):
    print("OPENVPN_PLUGIN_CLIENT_DISCONNECT\n")
    print_dict(env)

def PLUGIN_LEARN_ADDRESS(env):
    print("OPENVPN_PLUGIN_LEARN_ADDRESS\n")
    print_dict(env)
    '''
    # Uncomment this block for activate packet filter file creation
    if 'pf_file' in env:
        print('PF_File is %s'%(env['pf_file']))
        rules = ["[CLIENTS DROP]",
                "+fa56bf61-90da-11e8-bf33-005056a12a82-1234567",
                "+12345678-90da-11e8-bf33-005056a12a82-1234567",
                "[SUBNETS DROP]",
                "+10.150.0.1",
                "[END]"]
        with open(env['pf_file'], 'w') as f:
            f.write('\n'.join(rules))
    '''
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_CLIENT_CONNECT_V2(env):
    print("OPENVPN_PLUGIN_CLIENT_CONNECT_V2")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_TLS_FINAL(env):
    print("OPENVPN_PLUGIN_TLS_FINAL\n")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_ENABLE_PF(env):
    """
    Method OPENVPN_ENABLE_PF used for enable personal firewall rules for each client.
    If OPENVPN_ENABLE_PF is enabled, each called plug-ins part checks pf_file environment file
    """
    print("OPENVPN_PLUGIN_ENABLE_PF")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_ROUTE_PREDOWN(env):
    print("OPENVPN_PLUGIN_ROUTE_PREDOWN")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def PLUGIN_N(env):
    print("OPENVPN_PLUGIN_N")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

def OPENVPN_UNKNOWN_PLUGIN_TYPE(env):
    print("OPENVPN_UNKNOWN_PLUGIN_TYPE")
    print_dict(env)
    return OPENVPN_PLUGIN_FUNC_SUCCESS

