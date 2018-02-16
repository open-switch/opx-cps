#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

"""@package cps_utils

CPS Common Utilities

"""


import cps
import os
from os.path import isfile, join
import bytearray_utils
import socket
import re
import event_log as ev
from copy import deepcopy


module_path = "/usr/lib/python"

print_methods = {}
convert_methods = {}


def is_ipv4_addr(ip_addr):
    """
    Check if string is valid ipv4 address
    @ip_addr - ipv4 address string
    """
    try:
        socket.inet_pton(socket.AF_INET, ip_addr)
    except socket.error:
        return False
    return True


def is_ipv6_addr(ip_addr):
    """
    Check if string is valid ipv6 address
    @ip_addr - ipv6 address string
    """
    try:
        socket.inet_pton(socket.AF_INET6, ip_addr)
    except socket.error:
        return False
    return True


def is_mac_addr(mac_addr):
    """
    Check if string is a valid MAC address
    @mac_addr - MAC address string
    """
    if re.match("[0-9a-f]{2}([:])[0-9a-f]{2}(\\1[0-9a-f]{2}){4}$", mac_addr.lower()):
        return True
    return False


def is_int(val):
    """
    Check if string is valid integer value.
    @val - numeric string
    """
    if val.isdigit():
        return True
    return False

str_types_map = {
    is_ipv4_addr: 'ipv4',
                  is_ipv6_addr: 'ipv6',
                  is_mac_addr: 'mac',
                  is_int: 'uint32_t'
}


def get_string_type(val):
    for func in str_types_map:
        if func(val):
            return str_types_map[func]
    return 'string'
def _log_print(log, msg):
    if log == None:
        print msg
    else:
        ev.logging("DSAPI", log, "", "", "", 0, msg)


class CPSTypes:

    def __init__(self):
        self.types = {}

    def add_type(self, key, typ):
        self.types[key] = typ

    def guess_type_for_len(self, val):
        if isinstance(val, int):
            return 'uint32_t'
        if isinstance(val, str):
            return get_string_type(val)
        if isinstance(val, bytearray):
            if len(val) == 4:
                return 'uint32_t'
            if len(val) == 8:
                return 'uint64_t'
            if len(val) == 2:
                return 'uint16_t'
            if len(val) == 1:
                return 'uint8_t'
            if len(val) == 6:
                return 'mac'
        return None

    def find_type(self, key, val):
        if key in self.types:
            t = self.types[key]
        else:
            try:
                data_type = cps.type(key)
            except:
                data_type = {}

            if 'data_type' in data_type:
                t = data_type['data_type']
            else:
                t = self.guess_type_for_len(val)
            if t is None:
                t = 'bytearray'
        return t

    def from_data(self, key, val):
        t = self.find_type(key, val)
        return bytearray_utils.ba_to_value(t, val)

    def to_data(self, key, val):
        t = self.find_type(key, val)
        self.add_type(key, t)
        return bytearray_utils.value_to_ba(t, val)

    def print_list(self, attr_str, list, log=None):
        val_str = ""
        for item in list:
            if len(item) == 0:
                continue
            if len(val_str) > 0:
                val_str += ","
            val_str += str(self.from_data(attr_str, item))
        _log_print(log, (attr_str + " = " + val_str))

    def print_dict(self, data, log=None):
        for k in data:
            if k in print_methods:
                print_methods[k](data[k])
            elif type(data[k]) in print_methods:
                print_methods[type(data[k])](data[k])
            elif isinstance(data[k], list):
                self.print_list(k, data[k], log)
            elif isinstance(data[k], dict):
                self.print_dict(data[k], log)
            elif len(data[k]) == 0:
                _log_print(log, k + " = " + '-')
            else:
                _log_print(log, k + " = " + str(self.from_data(k, data[k])))

    def print_object(self, obj, log=None, show_key=True):
        data = obj['data']
        if show_key:
            _log_print(log, "Key: " + obj['key'])
        if len(data.keys()) == 0:
            return
        module = "/".join(data.keys()[0].split("/")[0:-1])
        if module in print_methods:
            print_methods[module](data)
        else:
            self.print_dict(data, log)

    def printable_list(self, attr_str, data):
        val_str = ""
        for item in data[attr_str]:
            if len(val_str) > 0:
                val_str += ","
            val_str += str(self.from_data(attr_str, item))
        data[attr_str] = val_str

    def printable_dict(self, data):
        for k in data:
            if isinstance(data[k], list):
                self.printable_list(k, data)
            elif isinstance(data[k], dict):
                self.printable_dict(data[k])
            else:
                if len(data[k]) == 0: data[k] = None
                else: data[k] = str(self.from_data(k, data[k]))

    def printable(self,obj):
        data = obj['data']
        if len(data.keys()) == 0:
            return
        self.printable_dict(data)



class CPSLibInit:

    def load_class_details(self):
        libs = []
        path = os.getenv("LD_LIBRARY_PATH")
        if path is None:
            path = '/opt/dell/os10/lib'
        for i in path.split(':'):
            print "Searching " + i
            files = os.listdir(i)
            for f in files:
                if f.find('cpsclass') == -1:
                    continue
                libs.append(i)
                break
        for i in libs:
            print "Loading from " + i
            res = cps.init(i, 'cpsclass')
            print res

    def __init__(self):
        self.load_class_details()


def init():
    CPSLibInit()


def key_mapper():
    return CPSKeys()

cps_attr_types_map = CPSTypes()


def add_print_function(type, func):
    """
    Register a custom print function for entire module, single attribute or
    a specific type to be called when the object is being printed via
    print_obj method of this package.
    @type - Any string (module_key/attribute_id/python data type)
    @func - function to be called when type is matched while printing
            the object
    """
    print_methods[type] = func


def add_convert_function(attr, func):
    """
    Register a function for pre processing the attribute value before
    adding it to a object. For ex. when adding a leaf-list value, value
    could be a string which has to be converted to list before it can be
    added to cps object.
    @attr - attribute id in string
    @func - function to be called before adding this attribute and its value
            to object
    """
    convert_methods[attr] = func


def add_attr_type(attr_str, dtype):
    """
    Add the type of the attribute id in the types map
    to convert attribute value to/from bytearray
    @attr_str - attribute id in string
    @dtype - data type of the attribute
    """
    cps_attr_types_map.add_type(attr_str, dtype)


def print_obj(obj,show_key=True):
    """
    Print the cps object in a user-friendly format
    @obj - cps object to be printed
    """
    if 'change' in obj:
        cps_attr_types_map.print_object(obj['change'], log=None,show_key=show_key)
    else:
        cps_attr_types_map.print_object(obj, log=None,show_key=show_key)

def log_obj(obj, log_level):
    """
    Log the cps object in a user-friendly format
    @obj - cps object to be logged
    @log_level - Log Level
    """
    if 'change' in obj:
        cps_attr_types_map.print_object(obj['change'], log_level)
    else:
        cps_attr_types_map.print_object(obj, log_level)


def printable(obj):
    """
        Make the object printable
    """
    if 'change' in obj:
        cps_attr_types_map.printable(obj['change'])
    else:
        cps_attr_types_map.printable(obj)

def get_modules_list():
    """
    Get the list of base python utlity modules.
    """
    file_list = [f.split(".")[0]
                 for f in os.listdir(module_path) if isfile(join(module_path, f))]
    file_set = set(file_list)
    return list(file_set)


from cps_object import *
from cps_operations import *
