

def to_string(s):
    s = s.replace('-','_')
    s = s.replace(':','_')
    s = s.replace('/','_')
    return s.upper();

def type_to_lang_type(self,typename):
    valid_types = {
       'boolean':'bool',
       'decimal64':'double',
       'int8':'int8_t',
       'int16':'int16_t',
       'int32':'int32_t',
       'int64':'int64_t',
       'uint8':'uint8_t',
       'uint16':'uint16_t',
       'uint32':'uint32_t',
       'uint64':'uint64_t',
       'string':'const char*',
       'binary':'uint8_t*'
    }
    if not typename in valid_types:
        return valid_types['binary'];
    return valid_types[typename]

class Language:
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
        for cont_key in self.map_with_keys.keys():
            for i in self.map_with_keys[cont_key].split():
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
        self.map_with_keys = {}
        for i in self.model.container_keys.keys():
            node = self.model.all_node_map[i]
            if node.find(self.model.module.ns()+'key')== None:
                continue
            self.map_with_keys[i] = self.model.container_keys[i]

        for i in self.map_with_keys.keys():
            new_key=""
            key_str = self.map_with_keys[i]
            for elem in key_str.split():
                new_key+=self.names[elem]+","

            new_key = new_key[:-1]
            self.keys[self.names[i]] = new_key
        self.determine_key_types()

    def __init__(self,context,model):
        self.context = context
        self.model = model
        self.names = {}
        self.keys = {}
        self.types = {}
        self.names[model.module.name()] = "cps_api_obj_CAT_"+to_string(model.module.name())
        self.handle_types()
        self.handle_enums()
        self.handle_container()
        self.handle_keys()

