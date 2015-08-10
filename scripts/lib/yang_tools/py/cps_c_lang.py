
import yin_utils
import cps_c_dict
import cps_h
import object_history
import sys

def to_string(s):
    s = s.replace('-','_')
    s = s.replace(':','_')
    s = s.replace('/','_')
    s = s.replace('+','_plus')
    s = s.replace('.','_dot')
    return s.upper();

valid_types = {
   'boolean':'bool',
   'decimal64':'double',
   'int8':'int8_t',
   'int16':'int16_t',
   'int3/2':'int32_t',
   'int64':'int64_t',
   'uint8':'uint8_t',
   'uint16':'uint16_t',
   'uint32':'uint32_t',
   'uint64':'uint64_t',
   'string':'const char*',
   'binary':'uint8_t*'
}

map_types = {
   'boolean':'CPS_CLASS_DATA_TYPE_T_BOOL',
   'decimal64':'CPS_CLASS_DATA_TYPE_T_DOUBLE',
   'int8':'CPS_CLASS_DATA_TYPE_T_INT8',
   'int16':'CPS_CLASS_DATA_TYPE_T_INT16',
   'int32':'CPS_CLASS_DATA_TYPE_T_INT32',
   'int64':'CPS_CLASS_DATA_TYPE_T_INT64',
   'uint8':'CPS_CLASS_DATA_TYPE_T_UINT8',
   'uint16':'CPS_CLASS_DATA_TYPE_T_UINT16',
   'uint32':'CPS_CLASS_DATA_TYPE_T_UINT32',
   'uint64':'CPS_CLASS_DATA_TYPE_T_UINT64',
   'string':'CPS_CLASS_DATA_TYPE_T_STRING',
   'binary':'CPS_CLASS_DATA_TYPE_T_BIN',
   'counter32' : 'CPS_CLASS_DATA_TYPE_T_UINT32',
   'zero-based-counter32' : 'CPS_CLASS_DATA_TYPE_T_UINT32',
   'counter64' : 'CPS_CLASS_DATA_TYPE_T_UINT64',
   'zero-based-counter64' : 'CPS_CLASS_DATA_TYPE_T_UINT64',
   'guage32' : 'CPS_CLASS_DATA_TYPE_T_UINT32',
   'guage64' : 'CPS_CLASS_DATA_TYPE_T_UINT64',
   'object-identifier' : 'CPS_CLASS_DATA_TYPE_T_OBJ_ID',
   'object-identifier-128' : 'CPS_CLASS_DATA_TYPE_T_OBJ_ID',
   'date-and-time' : 'CPS_CLASS_DATA_TYPE_T_DATE',
   'phy-address':'CPS_CLASS_DATA_TYPE_T_BIN',
   'mac-address':'CPS_CLASS_DATA_TYPE_T_BIN',
   'ip-version':'CPS_CLASS_DATA_TYPE_T_ENUM',
   'port-number':'CPS_CLASS_DATA_TYPE_T_UINT16',
   'ip-address':'CPS_CLASS_DATA_TYPE_T_IP',
   'ip4-address':'CPS_CLASS_DATA_TYPE_T_IPV4',
   'ip6-address':'CPS_CLASS_DATA_TYPE_T_IPV6',
   'ip-prefix':'CPS_CLASS_DATA_TYPE_T_IP',
   'ip4-prefix':'CPS_CLASS_DATA_TYPE_T_IPV4',
   'ip6-prefix':'CPS_CLASS_DATA_TYPE_T_IPV6',
   'domain-name':'CPS_CLASS_DATA_TYPE_T_STRING',
   'host':'CPS_CLASS_DATA_TYPE_T_STRING',
   'uri':'CPS_CLASS_DATA_TYPE_T_STRING',
   'enum':'CPS_CLASS_DATA_TYPE_T_ENUM',
   'enumeration':'CPS_CLASS_DATA_TYPE_T_ENUM',
   'union': 'CPS_CLASS_DATA_TYPE_T_BIN',
   'bits':'CPS_CLASS_DATA_TYPE_T_BIN',
   'empty':'CPS_CLASS_DATA_TYPE_T_BOOL',
   'leafref':'CPS_CLASS_DATA_TYPE_T_STRING',
}

def type_to_lang_type(typename):
    global valid_types
    if not typename in valid_types:
        return valid_types['binary'];
    return valid_types[typename]

class Language:
    def determine_type(self,node):
        return "cps_api_object_ATTR_T_BIN"

    def to_string(self,str):
        return to_string(str)

    def valid_lang_type(self,str):
        global valid_types
        return str in valid_types

    def cps_map_type(self,global_types,elem):
        global map_types
        type_str = self.get_type(elem)

        while type_str not in map_types and type_str in global_types:
            elem = global_types[type_str]
            type_str = self.get_type(elem)

        if not type_str in map_types:
            raise Exception("Failed to translate type...%s = %s" % (elem, type_str));

        return map_types[type_str]

    def type_to_lang_type(self,str):
        return type_to_lang_type(str)

    def get_type(self,node):
        type = node.find(self.model.module.ns()+'type')
        if type==None: return 'binary'
        return type.get('name')

    def determine_key_types(self):
        for cont_key in self.model.elem_with_keys.keys():
            for i in self.model.elem_with_keys[cont_key].split():
                path = i
                cname = self.names[i]

                type = self.get_type(self.model.all_node_map[i])


                if not type in self.context['types']:
                    self.types[cname] = "uint32"
                else:
                    node = self.context['types'][type]
                    self.types[cname] = self.get_type(node)
                self.types[cname] = type_to_lang_type(self.types[cname])


    def get_category(self):
        return self.category

    def init_names(self):
        for i in self.model.all_node_map:
            self.names[i] = self.to_string(i)
            self.names[self.names[i]] = i
        self.names[self.model.module.name()] = self.category

        for c in self.model.container_map[self.model.module.name()]:
            if c.name == self.model.module.name(): continue
            en_name = self.to_string(c.name+"_obj")
            self.names[c.name] = en_name

    def setup(self,model):
        self.model = model

        self.category = "cps_api_obj_CAT_"+to_string(model.module.name())

        self.init_names()

        hist_file_name = yin_utils.get_yang_history_file_name(model.filename)

        self.history = object_history.init(self.context,hist_file_name,self.category);

    def __init__(self,context):
        self.context = context
        self.names = {}
        self.keys = {}
        self.types = {}
        self.context['output']['header']['cps']=cps_h
        self.context['output']['src']['cps']=cps_c_dict

    def path_to_ids(self,module,path):
        array="}"
        while not path==None and not len(path)==0:
            if path in module.parent:
                par = module.parent[path]
            else:
                par = None
            array = str(module.name_to_id[path])+array
            if par !=None:
                array = ","+array
            path = par
        array = "{"+array
        return array


    def write(self):
        self.write_details('header')
        self.write_details('src')

    def write_details(self,type):
        class_type = self.context['output'][type]['cps']

        old_stdout = sys.stdout
        with open(self.context['args']['cps'+type],"w") as sys.stdout:
            class_type.COutputFormat(self.context).show(self.model)
        sys.stdout = old_stdout

    def close(self):
        self.history.write()