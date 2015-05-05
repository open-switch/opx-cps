
import yin_utils
import cps_c_dict
import cps_h
import object_history
import sys

def to_string(s):
    s = s.replace('-','_')
    s = s.replace(':','_')
    s = s.replace('/','_')
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


    def show_enum(self,name):
        node = self.model.context['enum'][name]
        enum = node.find('enumeration')

        for i in node.iter():
            if self.model.module.filter_ns(i.tag) == 'enum':
                self.names[name+"/"+i.get('name')]= to_string(name+"_"+i.get('name'))

        if not name in self.names:
            self.names[name] = to_string(name)+"_t";

    def handle_enums(self):
        name = self.model.module.name()
        for i in self.model.context['enum'].keys():
            if i.find(name)==-1: continue
            self.show_enum(i)

    def handle_container(self):
        for name in self.model.container_map.keys():
            if name == self.model.module.name(): continue

            node = self.model.container_map[name]
            if len(node)==0: continue

            for c in node:
                if c.name == self.model.module.name(): continue
                self.names[c.name] = to_string(c.name)

            if not name in self.names:
                self.names[name] = to_string(name)+"_t"

        if len(self.model.container_map[self.model.module.name()])==0:
            return

        subcat_name = self.model.module.name()+"_objects"

        for c in self.model.container_map[self.model.module.name()]:
            name = c.name
            node = self.model.container_map[name]
            self.names[name] = to_string(name+"_obj")

        if not subcat_name in self.names:
            self.names[subcat_name] = to_string(subcat_name)+"_t"

    def handle_types(self):
        for i in self.model.context['types'].keys():
            name = self.model.module.name()
            if i.find(name)==-1: continue

            if i in self.model.context['enum']: continue #already printed

            i = self.model.context['types'][i]
            if i.tag == self.model.module.ns()+'grouping': continue #not printable

            name = i.get('name');

            if not name in self.names:
                self.names[name] = to_string(name)+"_t"

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

    def handle_keys(self):
        for i in self.model.elem_with_keys.keys():
            new_key=""
            key_str = self.model.elem_with_keys[i]
            for elem in key_str.split():
                new_key+=self.names[elem]+","

            new_key = new_key[:-1]
            self.keys[self.names[i]] = new_key
        self.determine_key_types()

    def get_category(self):
        return self.category

    def setup(self,model):
        self.model = model

        self.category = "cps_api_obj_CAT_"+to_string(model.module.name())

        self.names[model.module.name()] = self.category

        hist_file_name = yin_utils.get_yang_history_file_name(model.filename)

        self.history = object_history.init(hist_file_name,self.category);

        self.handle_types()
        self.handle_enums()
        self.handle_container()
        self.handle_keys()

        self.setup_enums(model)


    def __init__(self,context):
        self.context = context
        self.names = {}
        self.keys = {}
        self.types = {}
        self.context['output']['header']['cps']=cps_h
        self.context['output']['src']['cps']=cps_c_dict

    def setup_enums(self,module):
        #alias
        history = self.history
        category = self.category

        for name in module.container_map.keys():
            if name == module.module.name(): continue
            node = module.container_map[name]
            if len(node)==0: continue

            for c in node:
                if c.name == module.module.name(): continue
                en_name = self.to_string(c.name)
                value = str(history.get_enum(en_name,None))
                module.name_to_id[c.name]= value

        if len(module.container_map[module.module.name()])==0:
            return

        subcat_name = module.module.name()+"_objects"
        for c in module.container_map[module.module.name()]:
            name = c.name
            node = module.container_map[name]
            en_name = self.to_string(name+"_obj")
            value = str(history.get_enum(en_name,None))
            module.name_to_id[c.name]= value

        id = history.get_global(category)
        module.name_to_id[category] = id
        #alias to module name too
        module.name_to_id[module.module.name()] = id

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