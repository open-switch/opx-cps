import sys


class Language:
    
    supported_list_containing_cb = [
        "container","grouping","choice", "list", "rpc","case"
        ]

    def __init__(self,context):
        self.context = context
    
    def change_prefix(self,name):
        loc = name.find(self.prefix)
        if loc!=0:
            print "Not found "+name
            return name
        name = self.module + name[len(self.prefix):]
        return name
        
    def name_to_cms_name(self,name):
        name = name.lower()
        name = name.replace('/','_')
        name = name.replace('-','_')
        name = name.replace(':','_')
        return name
        
    def setup_names(self):
        self.names = {}
        
        for k in self.model.all_node_map:
            name = self.name_to_cms_name(self.change_prefix(k))
            self.names[k] = name
            self.names[name] = k
            
    def find_cb_nodes(self):
        self.cb_nodes=[]
        for i in self.model.all_node_map:
            if self.model.all_node_map[i].tag in self.supported_list_containing_cb:
                self.cb_nodes.append(i)
        
        self.cb_node_keys = {}
        
        for k in self.cb_nodes:
            if k.find('/')!=-1:
                parent = self.model.parent[k]
                node = k[k.rfind('/'):]
                keys_raw = self.model.container_keys[parent]
            else:
                keys_raw = self.model.container_keys[k]
                node=""
                
            keys = ""
            keys_raw = keys_raw.split()
            keys_raw = keys_raw[1:]
            for str in keys_raw:
                keys+= self.names[str]+","
            if len(node)==0:
                keys = keys[:-1]
            else: keys+=self.names[k]

            self.cb_node_keys[self.names[k]]= keys
    
        self.cb_node_keys_data={}
        for k in self.cb_nodes:
            keys_raw = self.model.container_keys[k]
                
            keys = ""
            keys_raw = keys_raw.split()
            keys_raw = keys_raw[1:]
            for str in keys_raw:
                keys+= self.names[str]+" "
            
            self.cb_node_keys_data[self.names[k]]= keys
        
    
    def setup(self,model):
        self.model = model
        self.prefix = self.model.module.prefix
        self.module = self.model.module.module
        
        self.supported_list_containing_cb = self.model.module.prepend_ns_to_list(self.supported_list_containing_cb)
                
        self.setup_names()
        self.find_cb_nodes()
            
    def write_headers(self):
        print "#include \"cps_api_operation.h\""
        print "#include \"cma_utilities.h\" "
        print "#include \"cma_init.h\" "
        print "#include \""+self.module+"_xmltag.h\""
        print ""
    
    def get_keys(self,cb_node):
        keys = self.cb_node_keys_data[cb_node]
        line = ""
        for i in keys.split():
            print "  cma_value_t "+i+"_val;"
            print "  bool "+i+"_val_valid = cma_set_key_data(obj,"+i+",&"+i+"_val);"
            print ""
        
    
    def read_cb_node(self,cb_node):
        print "static cps_api_return_code_t _get_"+cb_node+" (void * context, cps_api_get_params_t * param, size_t key_ix) {"
        print ""
        print "  cps_api_object_t obj = cps_api_object_list_get(param->filters,key_ix);"
        print "  cps_api_key_t *key = cps_api_object_key(obj);"
        print ""
        self.get_keys(cb_node)
        print "  return cps_api_ret_code_OK;"
        print "}"
        print ""
    def write_cb_node(self,cb_node):
        print "cps_api_return_code_t _write_"+cb_node+"(void * context, cps_api_transaction_params_t * param, size_t key_ix) {"        
        print ""
        print "  cps_api_object_t obj = cps_api_object_list_get(param->change_list,key_ix);"
        print "  cps_api_object_t prev = cps_api_object_list_get(param->prev,key_ix);"
        print ""
        self.get_keys(cb_node)
        print "  return cps_api_ret_code_OK;"
        print "}"
        print ""
    def rollback_cb_node(self,cb_node):
        print "cps_api_return_code_t _rollback_"+cb_node+"(void * context, cps_api_transaction_params_t * param, size_t key_ix) {"        
        print ""
        print "  cps_api_object_t obj = cps_api_object_list_get(param->change_list,key_ix);"
        print "  cps_api_object_t prev = cps_api_object_list_get(param->prev,key_ix);"
        print ""
        self.get_keys(cb_node)
        print "  return cps_api_ret_code_OK;"
        print "}"
        print ""
        
    def write_all_cbs(self):
        for i in self.cb_node_keys:
            self.read_cb_node(i)
            self.write_cb_node(i)
            self.rollback_cb_node(i)
            print ""
    
    def write_init(self):
        print "void cma_init_"+self.module+"(void) {"
        print "  size_t ix = 0;"
        print "  size_t mx = "+str(len(self.cb_node_keys))+";"
        print "  for ( ; ix < mx ; ++ix ) {"
        for k in self.cb_node_keys:
            keys_list = self.cb_node_keys[k].split(',')
            two_keys = keys_list[:2]
            if len(two_keys)<2:
                two_keys+=" 0"
            rest_keys = keys_list[2:]
            
            print "     cps_api_registration_functions_t f;"
            print "     memset(&f,0,sizeof(f));"
            print "     "
            line=""
            line+="cps_api_key_init(&f.key,cps_api_qualifier_TARGET,"+','.join(two_keys)+","
            line+=str(len(rest_keys))
            for rk in rest_keys:
                line+=","+rk
            line+=");"
            print "     "+line
            print "     f._read_function=_get_"+k+";"
            print "     f._write_function=_set_"+k+";"
            print "     f._rollback_function=_rollback_"+k+";"
            print "     cma_init(&f,1);"
        print "  }"
        print"}"
    
    def write(self):
        
        old_stdout = sys.stdout
        with open(self.context['args']['cmssrc'],"w") as sys.stdout:
            self.write_headers()
            self.write_all_cbs()
            self.write_init()
            
        sys.stdout = old_stdout
        
        print ""
    
    def close(self):
        print ""