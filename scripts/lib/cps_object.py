#/usr/bin/python
#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN #AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
#  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

"""@package cps_object

CPS Object Utilities

Defines the CPS Object class. Can be accessed by importing module 'cps_utils'.

"""

''' CPS Object utilities'''

import cps
from copy import deepcopy
import cps_utils

types = cps_utils.cps_attr_types_map


class CPSObject:

    def __init__(self, module="", qual="target", data={}, obj={}):
        """
        Constructor to create a CPS object.
        @module - module key string, can be empty in case of
                  providing get object as parameter
        @qual - CPS qualifier (by default set to 'target')
        @data - data is a dictionary which can contain attr_str and
                values which will be added to object
        @obj  - object received from 'cps.get' function call, CPS object
                will be created from that object
        """

        self.obj = {'key': '', 'data': {}}
        self.root_path = ""
        self.embed_dict = {}

        if module:
            self.root_path = module + "/"
            self.obj['key'] = cps.key_from_name(qual, module)
            self.root_path = module + "/"

        for key, val in data.items():
            self.add_attr(key, val)

        if len(obj) > 0:
            if 'key' not in obj or 'data' not in obj:
                return None
            self.obj = deepcopy(obj)
            if len(obj['data'].keys()) > 0:
                for key in obj['data'].keys():
                    if key != 'cps/key_data':
                        self.root_path = "/".join(key.split("/")[:-1]) + "/"
                        return

        if not self.obj['key']:
            raise ValueError(
                "Invalid Module Name or object doesn't have the key")

    def set_key(self, key):
        """
        Change the key of the object.
        @key - CPS key string
        """
        self.obj['key'] = key

    def generate_path(self, attr_str):
        """
        Generates the full attribute path given an attribute string.
        In case of embedded attribute where attr_str could be a list, will keep
        appending the list string and return the full path.
        @attr_str - attribute string  or attribute string list
        """
        if "/" in attr_str:
            return attr_str
        elif isinstance(attr_str, list):
            ret_str = self.root_path
            for i in attr_str:
                if "/" in i:
                    ret_str = ""
                    break
            for i in attr_str:
                if not i.isdigit():
                    ret_str += i + "/"
            return ret_str[:-1]
        else:
            return self.root_path + attr_str

    def add_attr_type(self, attr_str, type):
        """
        Add a attribute type to accurately convert attribute value to and from
        bytearray. Only use this API for special data types such as 'ipv4', 'ipv6', 'hex'.
        Supported data types:
            string, hex, mac, ipv4, ipv6,
            uint8_t, unit16_t, uint32_t, uint64_t
        @attr_str - attribute string
        @type - type of the attribute
        """
        types.add_type(self.generate_path(attr_str), type)
        return

    def add_list(self, attr_str, val_list):
        """
        Add a list of values for a given attribute id string in the object.
        @attr_str - attribute string
        @val_list - list of values for given attribute string
        """
        ba_val_list = []
        if len(val_list) == 0:
            ba_val_list.append(bytearray(0))
        else:
            for val in val_list:
                ba_val_list.append(
                    types.to_data(self.generate_path(attr_str), val))
        self.obj['data'][self.generate_path(attr_str)] = ba_val_list

    def add_dict(self, attr_str, val_map):
        """
        Add values in dictionary for a given attribute id string in the object.
        @attr_str - attribute string
        @val_map - values with key for given attribute string
        """
        ba_val_map = {}
        root_path = self.generate_path(attr_str)
        for key in val_map:
            if isinstance(val_map[key], dict):
                sub_map = {}
                for sub_key in val_map[key]:
                    sub_path = root_path + '/' + sub_key
                    sub_map[sub_path] = types.to_data(sub_path, val_map[key][sub_key])
                ba_val_map[key] = sub_map
            else:
                ba_val_map[key] = types.to_data(key, val_map[key])
        self.obj['data'][self.generate_path(attr_str)] = ba_val_map

    def add_attr(self, attr_str, val):
        """
        Add a attribute and its value to the object.
        @attr_str - attribute string
        @val - value of the attribute
        """
        if self.generate_path(attr_str) in cps_utils.convert_methods:
            val = cps_utils.convert_methods[self.generate_path(attr_str)](val)

        if isinstance(val, list):
            self.add_list(attr_str, val)
        elif isinstance(val, dict):
            self.add_dict(attr_str, val)
        else:
            self.obj['data'][self.generate_path(attr_str)] = \
                types.to_data(self.generate_path(attr_str), val)

    def fill_data(self, data_dict):
        """
        Add attribute strings and its values from dictionary.
        @data_dict - dictionary which contains attribute string ids
                     and its values
        """
        for key, val in data_dict.items():
            self.add_attr(key, val)

    def add_embed_attr(self, attr_list, val, num_type_attrs=1):
        """
        Add Embedded attribute and its value to the object
        @attr_list - list of embedded attribute ids
        @val - value of the embedded attribute id
        @num_type_attrs - number of attributes from the end of attr_list that
                          are required to uniquely identify attr type
        """
        # Convert Values in bytearray
        attr_val = types.to_data(
                            self.generate_path(attr_list[-num_type_attrs:]),
                            val)

        # Find or create the parent container that holds the embedded final attr
        # Start with the container holding the complete obj data
        container = self.obj['data']
        partial_attrs = []

        for attr in attr_list[:-1]:
            attr_path = ''
            # if attr str is just a digit then treat it as a list index
            if attr.isdigit():
                attr_path = attr
            else:
                partial_attrs.append(attr)
                attr_path = self.generate_path(partial_attrs)

            if attr_path in container:
                container = container[attr_path]
            else:
                container[attr_path] = {}
                container = container[attr_path]

        container[self.generate_path(attr_list)] = attr_val

    def key_compare(self, key_dict):
        """
        Compare the attribute ids and its values given the key_dict to see if they
        match in the object. If attribute ids don't exist return true, if they exist and
        have the same value as in the key_dict return true otherwise return false
        @key_dict - dictionary which contains attribute ids and its values to be compared.
        """
        for key in key_dict:
            full_key = self.generate_path(key)
            if full_key in self.obj['data']:
                if key_dict[key] != types.from_data(full_key, self.obj['data'][full_key]):
                    return False
        return True

    def get(self):
        """
        Get the internal object which can be used with the CPS 'get' and transaction APIs.
        """
        return self.obj

    def get_key(self):
        """
        Get the key of the object.
        """
        return self.obj['key']

    def print_key_data(self):
        """
        Print the key attribute ids and its values for this object
        """

        keys = cps.get_keys(self.obj)
        if len(keys) > 0:
            for key, val in keys.items():
                print k + " = " + str(types.from_data(k, data[k]))
        elif 'cps/key_data' in self.obj['data']:
            for key, val in obj['data']['cps/key_data'].items():
                print k + " = " + str(types.from_data(k, data[k]))
        else:
            print "No keys found"

    def get_attr_data(self, attr):
        """
        Get the user readable attribute value for the given attribute id. If the attribute
        value exists in the obejct, return the value otherwise raise an value exception.
        @attr - attribute id whose value is to be returned
        """
        attr_path = self.generate_path(attr)
        l = []
        d = {}
        if attr_path in self.obj['data']:
            if isinstance(self.obj['data'][attr_path], list):
                for i in self.obj['data'][attr_path]:
                    if len(i) > 0:
                        l.append(types.from_data(attr_path, i))
                return l
            elif isinstance(self.obj['data'][attr_path], dict):
                for key,val in self.obj['data'][attr_path].items():
                    if isinstance(val, dict):
                        path_prefix = attr_path + '/'
                        prefix_len = len(path_prefix)
                        sub_map = {}
                        for sub_key,sub_val in val.items():
                            if len(sub_key) > prefix_len and sub_key[0:prefix_len] == path_prefix:
                                sub_path = sub_key[prefix_len:]
                            else:
                                sub_path = sub_key
                            sub_map[sub_path] = types.from_data(sub_key, sub_val)
                        d[key] = sub_map
                    else:
                        d[key] = types.from_data(key, val)
                return d
            return types.from_data(self.generate_path(attr),
                                   self.obj['data'][self.generate_path(attr)])

        if 'cps/key_data' in self.obj['data']:
            if attr_path in self.obj['data']['cps/key_data']:
                return types.from_data(attr_path,
                                       self.obj['data']['cps/key_data'][attr_path])

        raise ValueError(attr + "does not exist in the obect")

    def convert_to_ba_dict(self, data_dict):
        """
        Convert the dictionary of atribute-ids and its normal values to a dictionary
        which has full attribute-ids and its byte array values.
        @data_dict - dictionary which contains the attribute ids and its normal values.
        """
        converted_dict = {}
        for key, val in data_dict.items():
            converted_dict[self.generate_path(key)] = types.to_data \
                (self.generate_path(
                 key), val)
        return converted_dict


def clone(self, obj):
    """
    Clones a new object from a given object.
    @obj - object to clone
    """
    return deepcopy(obj)
