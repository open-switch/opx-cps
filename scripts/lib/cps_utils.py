import cps
import os
import bytearray_utils
import socket
import re

module_prefix_map = {
                    "stg" : "base-stg/entry",
                    "mirror" : "base-stg/entry",
                    "sflow" : "base-sflow/entry",
                    "lag" : "base-lag/entry",
                    "vlan" : "base-vlan/entry",
                    "acl-table" : "base-acl/table",
                    "acl-entry" : "base-acl/entry",
                    "mac" : "base-mac/table",
                    "physical" : "base-port/physical",
                    "interface" : "base-port/interface",
                    "route" : "base-route/obj/entry",
                    "switch" : "base-switch/switching-entities/switching-entity",
                    "interface-stat" : "base-stats/interfaces/interface",
                    "vlan-stat" : "base-stats/vlans/vlan"
                   }

def is_ipv4_addr(ip_addr):
    try:
        socket.inet_pton(socket.AF_INET,ip_addr)
    except socket.error:
        return False
    return True

def is_ipv6_addr(ip_addr):
    try:
        socket.inet_pton(socket.AF_INET6,ip_addr)
    except socket.error:
        return False
    return True

def is_mac_addr(mac_addr):
    if re.match("[0-9a-f]{2}([:])[0-9a-f]{2}(\\1[0-9a-f]{2}){4}$", mac_addr.lower()):
        return True
    return False

def is_int(val):
    if val.isdigit():
        return True
    return False

str_types_map = {
                  is_ipv4_addr : 'ipv4',
                  is_ipv6_addr : 'ipv6',
                  is_mac_addr  : 'mac',
                  is_int       : 'uint32_t'
                }

def get_string_type(val):
    for func in str_types_map:
        if func(val):
            return str_types_map[func]
    return 'string'


class CPSTypes:
    def __init__(self):
        self.types = {}

    def add_type(self, key, typ):
        self.types[key] = typ

    def guess_type_for_len(self,val):
        if type(val) == int:
            return 'uint32_t'
        if type(val) == str:
            return get_string_type(val)
        if type(val) == bytearray:
            if len(val)==4:
                return 'uint32_t'
        return None

    def find_type(self,key,val):
        if key in self.types:
            t = self.types[key]
        else:
            t = self.guess_type_for_len(val)
            if t==None:
                t = 'bytearray'
        return t

    def from_data(self, key, val):
        t = self.find_type(key,val)
        return bytearray_utils.ba_to_value(t,val)

    def to_data(self,key,val):
        t = self.find_type(key,val)
        return bytearray_utils.value_to_ba(t,val)

    def print_object(self,obj):
        data = obj['data']
        print "Key: "+obj['key']
        for k in data:
            print k +" = "+str(self.from_data(k,data[k]))

class CPSLibInit:
    def load_class_details(self):
        libs=[]
        path=os.getenv("LD_LIBRARY_PATH")
        if path==None:
            path='/opt/ngos/lib'
        for i in path.split(':'):
            print "Searching "+i
            files = os.listdir(i)
            for f in files:
                if f.find('cpsclass')==-1:continue
                libs.append(i)
                break;
        for i in libs:
            print "Loading from "+i
            res = cps.init(i,'cpsclass')
            print res

    def __init__(self):
        self.load_class_details()


def init():
    CPSLibInit()

def key_mapper():
    return CPSKeys()



def find_module(module_name):
    if "/" in module_name:
        return module_name
    if module_name in module_prefix_map:
        return module_prefix_map[module_name]
    for key,val in module_prefix_map.items():
        if module_name in val.split("/")[0]:
            return val



cps_attr_types_map = CPSTypes()


class CPSObject:

    def __init__(self,module,qual="target",op = "create",data={}):
        self.set_obj = {}
        self.root_path = ""
        self.embed_dict = {}
        obj = {'key':'','data':{}}
        self.set_obj['change'] = obj
        self.set_obj['operation'] = op
        module= find_module(module)
        self.root_path = module+"/"
        obj['key'] = cps.key_from_name(qual,module)
        for key,val in data.items():
            self.add_attr(key,val)

    def set_operation(self,op):
        self.set_obj['operation'] = op

    def generate_path(self, attr_str):
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

    def add_attr_type(self,attr_str,val):
        cps_attr_types_map.add_type(self.generate_path(attr_str),val)
        return

    def add_attr(self,attr_str,val):
         self.set_obj['change']['data'][self.generate_path(attr_str)] = \
            cps_attr_types_map.to_data(self.generate_path(attr_str),val)


    def fill_data(self,data_dict):
        for key,val in data_dict.items():
            self.add_attr(key,val)

    def add_embed_attr(self,attr_list,val):
        #Convert Values in bytearray
        attr_val = cps_attr_types_map.to_data(self.generate_path(attr_list),val)

        # Check if a nested dictioanry for first element in attr_list exist
        # if so then append to that dictioanry, otherwise create a new
        embed_dict = {}
        if self.generate_path(attr_list[0]) in self.set_obj['change']['data']:
            embed_dict = self.set_obj['change']['data'][self.generate_path(attr_list[0])]
        else:
            self.set_obj['change']['data'][self.generate_path(attr_list[0])] = embed_dict

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
        for key in key_dict:
            full_key = self.generate_path(key)
            if full_key in self.set_obj['change']['data']:
                if key_dict[key] != self.set_obj['change']['data'][full_key]:
                    return False
        return True

    def get(self):
        return self.set_obj

    def print_obj(self):
        cps_attr_types_map.print_object(self.set_obj['change'])


class CPSGetObject:

    def __init__(self,module,qual="target",filter = {}):
        self.get_obj = {}
        self.root_path = ""
        self.get_obj['data'] = {}
        module = find_module(module)
        self.get_obj['key'] = cps.key_from_name(qual,module)
        self.root_path = module+"/"
        for key,val in filter.items():
            self.add_filter(key,val)

    def generate_path(self, attr_str):
        if "/" in attr_str:
            return attr_str
        else:
            return self.root_path+attr_str

    def add_attr_type(self,attr_str,val):
        cps_attr_types_map.add_type(self.generate_path(attr_str),val)
        return

    def add_filter(self,attr_str,val):
        self.get_obj['data'][self.generate_path(attr_str)] = \
            cps_attr_types_map.to_data(self.generate_path(attr_str),val)

    def fill_filter(self,filter_dict):
        for key,val in filter_dict.items():
            self.add_filter(key,val)

    def key_compare(self,key_dict):
        for key in key_dict:
            full_key = self.generate_path(key)
            if full_key in self.get_obj['data']:
                if key_dict[key] != self.get_obj['data'][full_key]:
                    return False
        return True

    def print_obj(self):
        cps_attr_types_map.print_object(self.get_obj)

    def get(self):
        return self.get_obj
