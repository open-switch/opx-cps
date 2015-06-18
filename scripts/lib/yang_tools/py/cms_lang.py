import sys
import os

cma_gen_file_c_includes = """

#include \"cps_api_operation.h\"
#include \"cma_utilities.h\"
#include \"cma_init.h\"
#include \"cma_errnum.h\"


"""
write_statement_switch = """
  t_std_error retval = STD_ERR_OK;
  switch (edit_mode.phase) {
  case CMA_PH_VALIDATE:
     /* Validation phase code here */
     break;
  case CMA_PH_APPLY:
     /* Check whether ready for commit */
     break;
  case CMA_PH_COMMIT:
     /* Commit phase code here */
     switch (edit_mode.op) {
     case CMA_OP_LOAD:
         /*This is called at module startup to reload all data that module owns*/
         break;
     case CMA_OP_REPLACE:
         break;
     case CMA_OP_CREATE:
         /*Called when module is created*/
         break;
     case CMA_OP_MERGE:
         break;
     case CMA_OP_DELETE:
         break;
     default:
         retval = CMA_ERR_OPERATION_NOT_SUPPORTED;
     }
     break;
  case CMA_PH_ROLLBACK:
     /* Code for undoing commit here */
     break;
  default:
     retval = CMA_ERR_OPERATION_FAILED;
 }
"""

class Language:

    supported_list_containing_cb = [
        "container","grouping","case", "list", "rpc"
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
        self.names_short={}
        seen = {}
        for k in self.model.all_node_map:
            if k.rfind('/')!=-1:
                name = k[k.rfind('/')+1:]
            else:
                name = k
            to_add = self.name_to_cms_name(name)
            index=0
            while to_add in seen:
                to_add = name+str(index)
                index+=1
            name = to_add
            self.names_short[self.names[k]] = name

        self.cps_names = self.context['output']['language']['cps'].names


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
            if k not in self.model.container_keys:
                print "Missing "+k
                print  self.model.container_keys
                raise Exception("Missing key")
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

    def rw_access(self,yin_node):
        config_access = yin_node.find(self.model.module.ns()+'config')
        if config_access!=None:
            access = config_access.find('value')=='true'
            return access
        return True

    def is_parent_tree_read_only(self,cb_node):
        real_path = self.names[cb_node]
        while real_path.find('/')!=-1:
            node = self.model.all_node_map[real_path]
            if self.rw_access(node)==False: return True
            real_path = self.model.parent[real_path]
        return False

    def get_node_leaves_based_on_access(self,cb_node,read_only):
        full_name = self.names[cb_node]
        parent_ro = self.is_parent_tree_read_only(cb_node)

        ro_l = []
        rw_l = []

        for leaf in self.model.container_map[full_name]:
            leaf = leaf.name
            yin_node = self.model.all_node_map[leaf]
            if yin_node.tag != self.model.module.ns()+'leaf' and yin_node.tag != self.model.module.ns()+'leaf-list':
                continue
            if parent_ro or self.rw_access(yin_node)==False: ro_l.append(leaf)
            else: rw_l.append(leaf)

        if read_only: return ro_l
        return rw_l


    def node_rw_access(self,cb_node):
        full_name = self.names[cb_node]
        yin_node = self.model.all_node_map[full_name]
        if self.rw_access(yin_node)==False:
            return False
        l = self.get_node_leaves_based_on_access(cb_node,False)
        if len(l)==0: return False
        return True

    def get_keys(self,cb_node, object):
        keys = self.cb_node_keys_data[cb_node]
        line = ""
        print "/* Keys start... */ "
        for i in keys.split():
            full_name = self.names[i]
            yin_node = self.model.all_node_map[full_name]
            if yin_node.tag == self.model.module.ns()+'leaf' or yin_node.tag == self.model.module.ns()+'leaf-list':
                print "  cma_value_t "+self.names_short[i]+"_kval;"
                print "  const bool "+self.names_short[i]+"_kval_valid = cma_get_key_data("+object+","+i+",&"+self.names_short[i]+"_kval);"
                print "  (void)"+self.names_short[i]+"_kval_valid;"
                print ""

        print "/* Keys end... */ "

    def get_instance_vars(self,cb_node, read_only, function):
        full_name = self.names[cb_node]
        print "/* Instance vars start... */ "

        for leaf in self.get_node_leaves_based_on_access(cb_node,read_only):
            if function.find('set')!=-1:
                print "/*Update this field with the correct data before setting into object*/"
            print "  cma_value_t "+self.names_short[self.names[leaf]]+"_val;"
            print "  const bool "+self.names_short[self.names[leaf]]+"_val_valid = "+function+"(obj,"+self.names[leaf]+",&"+self.names_short[self.names[leaf]]+"_val);"
            if function.find('get')!=-1:
                print "  /* Check to see if the attribute exists in the object and set it into your internal data structure */"
            print "  (void)"+self.names_short[self.names[leaf]]+"_val_valid;"
            print ""
        print "/* Instance vars end... */ "


    def read_cb_node(self,cb_node):
        print "static cps_api_return_code_t _get_"+cb_node+" (void * context, cps_api_get_params_t * param, size_t key_ix) {"
        print ""
        print "  cps_api_object_t filter = cps_api_object_list_get(param->filters,key_ix);"
        print "  cps_api_key_t *key    = cps_api_object_key(filter);"
        print "  cps_api_object_t obj = cps_api_object_create();"
        print ""
        print "  if (obj==NULL) return cps_api_ret_code_ERR;"
        print "  if (!cps_api_object_list_append(param->list,obj)) {"
        print "     cps_api_object_delete(obj);"
        print "     return cps_api_ret_code_ERR;"
        print "  }"
        print ""
        print "  cps_api_key_copy(cps_api_object_key(obj),key);"
        self.get_keys(cb_node,"filter")
        print ""
        self.get_instance_vars(cb_node,True,"cma_set_data")
        print ""
        print " /*Return a cps_api_ret_code_OK when you implement and have a successful operation*/"
        print "  return cps_api_ret_code_ERR;"
        print "}"
        print ""
    def write_cb_node(self,cb_node):
        print "cps_api_return_code_t _set_"+cb_node+"(void * context, cps_api_transaction_params_t * param, size_t key_ix) {"
        print ""
        print "  /*object that contains the data to set*/ "
        print "  cps_api_object_t obj = cps_api_object_list_get(param->change_list,key_ix);"
        print "  cps_api_object_t prev = cps_api_object_list_get(param->prev,key_ix); (void)prev;"
        print "  cma_edit_mode_t edit_mode;"
        print "  cma_edit_mode(obj,&edit_mode);"

        print ""
        self.get_keys(cb_node,"obj")
        print ""
        self.get_instance_vars(cb_node,False,"cma_get_data")

        print""
        print write_statement_switch
        print ""
        print "  (void)retval;"
        print " /*Return a cps_api_ret_code_OK when you implement and have a successful operation"
        print "   also can return a module specific return code.*/"
        print "  return cps_api_ret_code_ERR;"
        print "}"
        print ""


    def write_init(self, elem,read_res, write_res ):
        print "void cma_init_"+elem+"(void) {"
        print "  cps_api_registration_functions_t f;"
        keys_list = self.cb_node_keys[elem].split(',')
        two_keys = keys_list[:2]
        if len(two_keys)<2:
            two_keys.append("0")
        rest_keys = keys_list[2:]

        print "  memset(&f,0,sizeof(f));"
        print "  "
        line=""
        line+="cps_api_key_init(&f.key,cps_api_qualifier_TARGET,"+','.join(two_keys)+","
        line+=str(len(rest_keys))
        for rk in rest_keys:
            line+=","+rk
        line+=");"
        print "  "+line
        if read_res:  print "  f._read_function=_get_"+elem+";"
        else:         print "  f._read_function=NULL;"
        if write_res: print "  f._write_function=_set_"+elem+";"
        else:         print "  f._write_function=NULL;"
        print "  f._rollback_function=NULL;"
        print "  cma_api_init(&f,1);"
        print"}"

    def write_headers(self, elem):
        print "/*Generated for "+elem+"*/"
        print cma_gen_file_c_includes
        print "#include \""+self.module+"_xmltag.h\""

    def xmltag_mapping(self):
            print "/* OPENSOURCELICENSE */"
            print "#ifndef __"+self.name_to_cms_name(self.module)+"_xmltag_cma_h"
            print "#define __"+self.name_to_cms_name(self.module)+"_xmltag_cma_h"
            print "#include \""+self.module+".h\""
            print "/*Autogenerated file from "+self.module+"*/"
            print ""
            for i in self.names:
                if i in self.model.all_node_map and self.model.is_id_element(self.model.all_node_map[i]):
                    print "#define "+self.names[i]+" "+self.cps_names[i]+" "

            print "#endif"

    def xmltag_init_mapping_hdr(self):
        print "/* OPENSOURCELICENSE */"
        print "#ifndef __"+self.name_to_cms_name(self.module)+"_init_mapping_xmltag_h"
        print "#define __"+self.name_to_cms_name(self.module)+"_init_mapping_xmltag_h"

        print """

#ifdef __cplusplus
#include <unordered_map>
#include <string>
#include <stdint.h>

void init_"""+self.name_to_cms_name(self.module)+"""_xmltag(std::unordered_map<std::string,uint32_t> &ref);

#endif
#endif
"""
        
    def xmltag_mapping_src(self):
            print "/* OPENSOURCELICENSE */"
            print "#include \""+self.module+".h\""
            print "#include \""+self.module+"_xmltag.h\""
            print ""
            print "/*Autogenerated file from "+self.module+"*/"
            str = """
#include <string>
#include <unordered_map>

using un_name_to_id = std::unordered_map<std::string,uint32_t>;
static un_name_to_id _map = {
            """
            print str
            for i in self.names:
                if i in self.model.all_node_map and self.model.is_id_element(self.model.all_node_map[i]):
                    print "{\""+self.names[i]+"\","+self.cps_names[i]+"},"

            print "};"
            print ""
            print """
void init_"""+self.name_to_cms_name(self.module)+"""_xmltag(std::unordered_map<std::string,uint32_t> &ref) {
    ref.insert(_map.begin(),_map.end());
}

"""


    def write(self):
        old_stdout = sys.stdout

        with open(os.path.join(self.context['args']['cmsheader'],self.module+"_xmltag.h"),"w") as sys.stdout:
            self.xmltag_mapping()

        with open(os.path.join(self.context['args']['cmsheader'],self.module+"_init_mapping_xmltag.h"),"w") as sys.stdout:
            self.xmltag_init_mapping_hdr()

        with open(os.path.join(self.context['args']['cmssrc'],self.module+"_xmltag.cpp"),"w") as sys.stdout:
            self.xmltag_mapping_src()



        for elem in self.cb_node_keys:
            with open(os.path.join(self.context['args']['cmssrc'],elem+".h"),"w") as sys.stdout:
                print "/* OPENSOURCELICENSE */"
                print "#ifndef __"+elem+"_cma_h"
                print "#define __"+elem+"_cma_h"
                print "#ifdef __cplusplus"
                print "extern \"C\" {"
                print "#endif"
                print "/*Autogenerated file from "+self.module+"*/"
                print "void cma_init_"+elem+"(void) ;"
                print ""
                print "#ifdef __cplusplus"
                print "}"
                print "#endif"
                print ""
                print "#endif"


        for elem in self.cb_node_keys:
            with open(os.path.join(self.context['args']['cmssrc'],elem+".c"),"w") as sys.stdout:
                self.write_headers(elem)
                read_res = False
                ro_node = self.node_rw_access(elem)==False
                rw_elems = len(self.get_node_leaves_based_on_access(elem,False))!=0
                ro_elems = len(self.get_node_leaves_based_on_access(elem,True))!=0

                if ro_node or ((not ro_node) and ro_elems):
                    self.read_cb_node(elem)
                    read_res = True

                write_res = False

                if (not ro_node) and rw_elems:
                    self.write_cb_node(elem)
                    write_res = True

                self.write_init(elem,read_res, write_res )
                print ""
        sys.stdout = old_stdout

    def close(self):
        print ""
