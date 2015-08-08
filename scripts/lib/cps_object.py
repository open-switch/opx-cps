#/usr/bin/python

import cps
from copy import deepcopy
import cps_utils

types = cps_utils.cps_attr_types_map

class CPSObject:

    def __init__(self,module="",qual="target",data={},obj={}):
        """
        Constructor to create CPS object
        @module - module key string, can be empty in case of
                  passing get object as parameter
        @qual - qulifier by default is target
        @data - data is a dictionary which can contain attr_str and
                values which would be added to objectimpor
        @obj  - object received from cps.get function call, CPSobject
                would be created from that object
        """

        self.obj = {'key':'','data':{}}
        self.root_path = ""
        self.embed_dict = {}

        if module:
            self.root_path = module+"/"
            self.obj['key'] = cps.key_from_name(qual,module)
            self.root_path = module + "/"

        for key,val in data.items():
            self.add_attr(key,val)

        if len(obj) > 0:
            if 'key' not in obj or 'data' not in obj:
                return None
            self.obj = deepcopy(obj)
            if len(obj['data'].keys()) > 0:
                for key in obj['data'].keys():
                    if key != 'cps/key_data':
                        self.root_path = "/".join(key.split("/")[:-1])+"/"
                        return

        if not self.obj['key']:
            raise ValueError ("Invalid Module Name or object doesn't have the key")


    def set_key(self,key):
        """
        Change the key of the object
        @key - cps key string
        """
        self.obj['key']=key

    def generate_path(self, attr_str):
        """
        Generates the full attribute path given just the attribute string
        In case of embedded attribute where attr_str could be list, will keep
        appending the list string and return the full path
        @attr_str - attribute string  or attribute string list
        """
        if "/" in attr_str:
            return attr_str
        elif type(attr_str) == list:
            ret_str = self.root_path
            for i in attr_str:
                if not i.isdigit():
                    ret_str += i +"/"
            return ret_str[:-1]
        else:
            return self.root_path+attr_str

    def add_attr_type(self,attr_str,type):
        """
        Add a attribute type to accurately convert attribute value to and from
        bytearray. Only use this API for special data types like ipv4, ipv6, hex.
        Supported data type - string, hex, mac, ipv4, ipv6, uint8_t, unit16_t,
        uint32_t, uint64_t
        @attr_str - attribute string
        @type - type of the attribute
        """
        types.add_type(self.generate_path(attr_str),type)
        return

    def add_list(self,attr_str,val_list):
        """
        Add a list of values for a given attribute id string in the object
        @attr_str - attribute string
        @val_list - list of values for given attribute string
        """
        ba_val_list = []
        for val in val_list:
            ba_val_list.append(types.to_data(self.generate_path(attr_str),val))
        self.obj['data'][self.generate_path(attr_str)] = ba_val_list

    def add_attr(self,attr_str,val):
        """
        Add a attribute and its value to object
        @attr_str - attribute string
        @val - value of the attribute
        """
        if self.generate_path(attr_str) in cps_utils.convert_methods:
            val = cps_utils.convert_methods[self.generate_path(attr_str)](val)

        if type(val) == list:
            self.add_list(attr_str,val)
        else:
            self.obj['data'][self.generate_path(attr_str)] = \
            types.to_data(self.generate_path(attr_str),val)

    def fill_data(self,data_dict):
        """
        Add attribute strings and its values from dictionary
        @data_dict - dictionary which contains attribute string ids
                     and its values
        """
        for key,val in data_dict.items():
            self.add_attr(key,val)

    def add_embed_attr(self,attr_list,val):
        """
        Add Embedded attribute and its value to the object
        @attr_list - list of embedded attribute ids
        @val - value of the embedded attribute id
        """
        #Convert Values in bytearray
        attr_val = types.to_data(self.generate_path(attr_list[-1]),val)

        # Check if a nested dictioanry for first element in attr_list exist
        # if so then append to that dictioanry, otherwise create a new
        embed_dict = {}
        if self.generate_path(attr_list[0]) in self.obj['data']:
            embed_dict = self.obj['data'][self.generate_path(attr_list[0])]
        else:
            self.obj['data'][self.generate_path(attr_list[0])] = embed_dict

        for attr in reversed(attr_list[1:]):
            obj = {}
            # if a string is a digit treat is as a list index
            # if index exist in dictionary append to it, or add first key
            if attr.isdigit():
                if attr in embed_dict:
                    exsisting_nested_dict = embed_dict[attr]
                    exsisting_nested_dict[attr_val.keys()[0]] = attr_val.values()[0]
                else:
                    embed_dict[attr] = attr_val
            else:
                obj[self.generate_path(attr_list)] = attr_val
            attr_val = obj


    def key_compare(self,key_dict):
        """
        Compare the attribute ids and its values given the key_dict to see if they
        match in the object. If attribute ids don't exist return true, if they exist and
        have the same value as in the key_dict return true otherwise return false
        @key_dict - dictioanry which contains attribute ids and its values to be compared.
        """
        for key in key_dict:
            full_key = self.generate_path(key)
            if full_key in self.obj['data']:
                if key_dict[key] != types.from_data(full_key,self.obj['data'][full_key]):
                    return False
        return True

    def get(self):
        """
        Get the internal object which can be used with the cps get and transaction apis
        """
        return self.obj

    def get_key(self):
        """
        Get the key from the object
        """
        return self.obj['key']

    def print_key_data(self):
        """
        Print the key attribute ids and its values for this object
        """

        keys = cps.get_keys(self.obj)
        if len(keys) > 0:
            for key,val in keys.items():
                print k +" = "+str(types.from_data(k,data[k]))
        elif 'cps/key_data' in self.obj['data']:
            for key,val in obj['data']['cps/key_data'].items():
                print k +" = "+str(types.from_data(k,data[k]))
        else:
            print "No keys found"


    def get_attr_data(self,attr):
        """
        Get the user readable attribute value for the given attribute id. If the attribute
        value exist in the obejct, return the value otherwise raise an value exceprtion
        @attr - attribute id whose value is to be returned
        """
        attr_path = self.generate_path(attr)
        l = []
        if attr_path in self.obj['data']:
            if type(self.obj['data'][attr_path]) == list:
                for i in self.obj['data'][attr_path]:
                    l.append(types.from_data(attr_path,i))
                return l
            return types.from_data(self.generate_path(attr),\
                                   self.obj['data'][self.generate_path(attr)])

        if 'cps/key_data' in self.obj['data']:
            if attr_path in self.obj['data']['cps/key_data']:
                return types.from_data(attr_path,\
                       self.obj['data']['cps/key_data'][attr_path])

        raise ValueError(attr + "does not exist in the obect")

    def convert_to_ba_dict(self,data_dict):
        """
        Convert the dictionary of atribute-ids and its normal values to a dictioanry
        which has full attribute-ids and its byte array values
        @data_dict - dictaionry which contains the attribute ids and its normal values
        """
        converted_dict = {}
        for key,val in data_dict.items():
            converted_dict[self.generate_path(key)] = types.to_data \
                                                   (self.generate_path(key),val)
        return converted_dict

def clone(self,obj):
    """
    Clone a new object from given object
    """
    return deepcopy(obj)
