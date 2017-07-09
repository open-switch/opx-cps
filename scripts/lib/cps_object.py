#/usr/bin/python
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

"""@package cps_object

CPS Object Utilities

Defines the CPS Object class. Can be accessed by importing module 'cps_utils'.

"""

''' CPS Object utilities'''

import cps
from copy import deepcopy
import cps_utils
import argparse

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
        self.properties = {}

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

    def set_property(self,name,value):
        """
        Store some additional meta properties in the object that will not be used in the backend but can be used
        for other application level development

        Eg.. application scratch pad
        @param name is the name of the attribute
        @param value is the value for the attribute
        """
        if value is None:
            del self.properties[name]
        else:
            self.properties[name] = value

    def get_property(self,name):
        """
        Check the object and see if the propery exists. If exsts, return it otherwise return None
        """
        if name in self.properties:
            return self.properties[name]
        return None

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

        if val == None:
            self.obj['data'][self.generate_path(attr_str)] = bytearray(0)
            return

        if isinstance(val, list):
            self.add_list(attr_str, val)
        elif isinstance(val, dict):
            self.add_dict(attr_str, val)
        else:
            self.obj['data'][self.generate_path(attr_str)] = \
                types.to_data(self.generate_path(attr_str), val)

    def del_attr(self, attr_str):
        """
        Delete an attribute and its value from the object.
        @attr_str - attribute string
        """
        self.obj['data'].pop(self.generate_path(attr_str),None)

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
            elif len(self.obj['data'][attr_path]) == 0:
                return None
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

    def set_error_string(self, return_code, msg, *args ):
		"""
		This function will set the error string and erro code within an object.
		@return_code the return code being set in the object
		@msg is the string formatting
		@*args is the variable length list of parameters to the formating
		"""
		self.add_attr("cps/object-group/return-code",return_code)
		_str = msg.format(*args)
		self.add_attr("cps/object-group/return-string",_str)
		
    def set_wildcard(self,enabled):
    	"""
    	This function will set the wildcard attribute within an object to the value specified
    	@enabled is the boolean value to set as wildcard (eg.. True)
    	"""
    	self.add_attr('cps/object-group/wildcard-search',enabled)

    def set_exact_match(self,use_exact_match):
    	"""
    	This function will set the exact match attribute within an object triggering behaviour
    	that will use the attributes within the object to search/monitor events.
    	@use_exact_match a boolean value that will be True if exact match filter is needed or false if not    			
    	"""    	
    	self.add_attr('cps/object-group/exact-match',use_exact_match)

    def set_number_of_entries(self,count):
        """
        This function will set the number of entries attribute within an object to the specified value
        @count number of entries required to be set
        """
        self.add_attr('cps/object-group/number-of-entries',count)

    	
def clone(self, obj):
    """
    Clones a new object from a given object.
    @obj - object to clone
    """
    return deepcopy(obj)

def object_from_parameters(prog,description, optional_fields=[]):
    """
    Uses Argparse to parse the program's command line arguments into an object.
    @param prog the name of the program (passed into argparse)
    @param description of the program (also passed to argparse)
    @param list_of_required_fields is used to determine which field is mandatory/not required valid values include "mod","qual","oper","attr"
    """

    _qualifiers = cps.QUALIFIERS

    parser = argparse.ArgumentParser(prog=prog, description=description)

    parser.add_argument('module',help='The object\'s name and optional qualifier.  For instance: cps/node-group or if/interfaces/interface.  A qualifier can optionally be placed at the beginning')
    parser.add_argument('additional',help='This field can contain a series of object attributes in the form of attr=value combinations',action='append',nargs='*')

    parser.add_argument('-mod',help='An alternate way to specify the module name', metavar='module',required=False,action='store',nargs=1)
    parser.add_argument('-d',help='Print some additional details about the objects parsed and sent to the backend', required=False, action='store_true')

    parser.add_argument('-qua',choices=cps.QUALIFIERS,help='The object\'s qualifier',required=False,action='store')
    parser.add_argument('-attr',help='Object attributes in the form of attr=value', required=False,action='append')

    parser.add_argument('-db',help='Attempt to use the db directly to satisfy the request instad of the normally registered object', required=False,action='store_true')

    if 'oper' in optional_fields :
        parser.add_argument('-oper',choices=cps.OPERATIONS, help='The operation types.  This is only used in CPS commit operations', required=True,action='store')

    if 'commit-event' in optional_fields :
        parser.add_argument('-commit-event',help='This flag will try to force the default state of the auto-commit event to true.  This is only used in CPS commit operations', required=False,action='store_true')

    _args = vars(parser.parse_args())
    if 'd' in _args and _args['d']:
        print _args

    _qual_list = cps.QUALIFIERS
    _class_name = _args['module']
    _attrs=[]

    qual = _qual_list[0]

    if len(_class_name) > 0:
        _lst = _class_name.split(cps.PATH_SEP,1);

        if len(_lst)>0 and _lst[0] in _qual_list:
            _class_name = _lst[1]
            qual = _lst[0]

    if 'additional' in  _args and _args['additional']!=None:
        _lst = _args['additional'][0]
        for i in range(0,len(_lst)):
            if _lst[i] in _qual_list:
                qual = _lst[i]
            else:
                _attrs.append(_lst[i])

    if 'attr' in _args and _args['attr']!=None :
        for i in _args['attr']:
            _attrs.append(i)

    if 'qua' in _args and _args['qua']!=None:
        qual = _args['qua']

    if len(_class_name)>0 and _class_name[len(_class_name)-1]=='/':
        _class_name = _class_name[0:-1]

    obj = CPSObject(_class_name,qual)

    if 'oper' in _args and _args['oper']!=None:
        if 'd' in _args and _args['d']: print('Operation type is: %s' %_args['oper'])
        obj.set_property('oper',_args['oper'])

    for i in _attrs:
        if i.find('=')==-1:
            continue
        _data = i.split('=', 1)
        # For embedded attribute check if comma seperated attribute list is given
        # then add it as embedded
        embed_attrs = _data[0].split(',')
        if len(embed_attrs) == 3:
            obj.add_embed_attr(embed_attrs,_data[1])
        else:
            obj.add_attr(_data[0],_data[1])

    if 'db' in _args and _args['db']:
        if 'd' in _args and _args['d']:
            print('Attempt to force database use')
        cps.set_ownership_type(obj.get_key(),'db')
        obj.set_property('db',True)
    else:
        obj.set_property('db',False)

    if 'commit_event' in _args and _args['commit_event']:
        if 'd' in _args and _args['d']:
            print('Attempt to force use of auto-event for requested change on %s' % obj.get_key())

        cps.set_auto_commit_event(obj.get_key(),True)
        obj.set_property('commit-event',True)

    if 'd' in _args and _args['d']:
        print obj.get()
    return obj
