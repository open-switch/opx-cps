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
        self.print_methods = {}

    def add_print_method(self,type,func):
        self.print_methods[type] = func

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
            if type(k) in self.print_methods:
                    self.print_methods[type(k)](k)
            else:
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

def add_print_function(type,func):
    cps_attr_types_map.add_print_method(type,func)

def print_obj(obj):
    if 'change' in obj:
        cps_attr_types_map.print_object(obj['change'])
    else:
        cps_attr_types_map.print_object(obj)

class CPSTransaction:
    def __init__(self):
        self.tr_list = []

    def create(self,obj):
        tr_obj = {}
        tr_obj['change'] = obj
        tr_obj['operation'] = "create"
        self.tr_list.append(tr_obj)

    def delete(self,obj):
        tr_obj = {}
        tr_obj['change'] = obj
        tr_obj['operation'] = "delete"
        self.tr_list.append(tr_obj)

    def set(self,obj):
        tr_obj = {}
        tr_obj['change'] = obj
        tr_obj['operation'] = "set"
        self.tr_list.append(tr_obj)

    def commit(self):
        if cps.transaction(self.tr_list):
            return self.tr_list
        return False


class CPSObject:

    def __init__(self,module,qual="target",data={}):
        self.obj = {'key':'','data':{}}
        self.root_path = ""
        self.embed_dict = {}
        module= find_module(module)
        self.root_path = module+"/"
        self.obj['key'] = cps.key_from_name(qual,module)
        for key,val in data.items():
            self.add_attr(key,val)

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
         self.obj['data'][self.generate_path(attr_str)] = \
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
        for key in key_dict:
            full_key = self.generate_path(key)
            if full_key in self.obj['data']:
                if key_dict[key] != self.obj['data'][full_key]:
                    return False
        return True

    def get(self):
        return self.obj

    def print_key_data(self):
        if 'cps/key_data' in self.obj['data']:
            for key,val in obj['data']['cps/key_data'].items():
                print k +" = "+str(cps_attr_types_map.from_data(k,data[k]))


def cps_create_transaction_object(op,qual,module):
    """
    Create a cps object for performing transaction
    @op = operation type ("create","delete","set")
    @qual = qualifier type ("target","observed",..)
    @module = module key string ("base-xxx/yyy")
    @return a cps object
    """
    cps_op = {}
    obj = {'key':'','data':{}}
    cps_op['change'] = obj
    cps_op['operation'] = op
    cps_op['root_path'] = module+"/"
    obj['key'] = cps.key_from_name(qual,module)
    return cps_op


def cps_generate_attr_path(cps_object, attr_str):
    if "/" in attr_str:
        return attr_str
    else:
        return cps_object['root_path']+attr_str


def cps_add_attr_type(cps_object,attr_str,val):
    cps_attr_types_map.add_type(cps_generate_attr_path(cps_object,attr_str),val)


def cps_object_add_attr(cps_object,attr_str,val):
    """
    Add Attributes to cps object
    @cps_object = cps object
    @attr_str = attr string in yang("id", "base-port/interface/id")
    @val = value of attribute
    @return none
    """
    cps_object['change']['data'][cps_generate_attr_path(cps_object,attr_str)] = \
    cps_attr_types_map.to_data(cps_generate_attr_path(cps_object,attr_str),val)


def cps_create_get_object(qual,module):
    """
    Create a cps get operation object
    @qual = qualifier type ("target","observed",..)
    @module = module string ("base-xxx/yyy")
    @return a cps get object with keys populated
    """
    obj = {'key':'','data':{}}
    obj['key'] = cps.key_from_name(qual,module)
    obj['root_path'] = module+"/"
    return obj


def cps_object_add_filter(cps_get_object,attr_str,val):
    """
    Add Filter Attributes to cps_get_object
    @cps_get_object = cps get object
    @attr_str = attr string in yang("id", "port")
    @val = value of attribute
    @return a dictioanry with all cps data populated
    """
    cps_get_object['data'][cps_generate_attr_path(cps_get_object,attr_str)] = \
    cps_attr_types_map.to_data(cps_generate_attr_path(cps_get_object,attr_str),val)


def cps_obj_key_compare(cps_api_obj, key_dict):
    """
    Find if the keys in key dictionary are present in the cps obj
    and has the same value
    @cps_api_obj - cps transcation object
    @key_dict - dictionary which has keys and values
    @return - True if all keys and its values are same in the obj,
              false if key is matched but not its value
    """
    for key in key_dict:
        if key in cps_api_obj['change']['data']:
            if key_dict[key] != cps_api_obj['change']['data'][key]:
                return False
    return True

