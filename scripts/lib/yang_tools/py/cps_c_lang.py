
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

        self.history = object_history.init(hist_file_name,self.category);

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