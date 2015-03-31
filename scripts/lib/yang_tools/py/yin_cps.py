
import yin_ns
import yin_utils
import sys
import object_history
import yin_enum
import os

import xml.etree.ElementTree as ET

import tempfile


supported_ids_at_root = [
    "list","container" ]


supported_list_containing_children = [
    "container","grouping","choice", "list", "rpc" , "case", "module"]


supported_list_of_leaves_have_attr_ids = [
    "container","grouping","choice", "list", "leaf","leaf-list", "rpc", "uses" ]

def get_id_of_node(history, parent, node):
    en = history.get_enum(yin_utils.string_to_c_formatted_name(parent))
    return en.get_value(yin_utils.string_to_c_formatted_name(node))


class CPSId:
    name = ""
    cps_id = ""
    node = None
    path = ""
    type = ""
    parent = None
    
    def __init__(self, cps_id, name, node, path, t=None):
        self.name = name
        self.cps_id = cps_id
        self.node = node
        self.path = path
        self.type = t

    def show(self) :
        str_id = str(self.cps_id)
        print self.path+" CPS ID("+str_id+") = "+yin_utils.string_to_c_formatted_name(self.name)

    def to_string(self,module):
        s = yin_utils.string_to_c_formatted_name(self.name) + " = " + str(self.cps_id)
        if self.type == None:
            self.type = yin_utils.node_get_type(module,self.node)
        s+= "/*!< "+self.type+" */"
        return s;


class CPSContainer:
    list_of_ids = list()
    name = ""
    node_path = ""
    node = None
    module = None
    def __init__(self,module,name,path,node):
        self.module = module
        self.list_of_ids = list()
        self.name = name
        self.path =path
        self.node = node

    def add(self,cps_id):
        self.list_of_ids.append(cps_id)

    def show(self):
        print self.path + " Container -> " + yin_utils.string_to_c_formatted_name(self.name)+"_T"
        for i in self.list_of_ids:
            i.show()

    def to_string(self) :
        n =  yin_utils.string_to_c_formatted_name(self.name)+"_t"

        if len(self.list_of_ids) == 0:
            return "/* "+n+"*/\n"
        desc = yin_utils.node_get_desc(self.module,self.node)
        s = ""
        if len(desc) > 0:
            s += "/* "+desc+ " */\n"

        s += "typedef enum {\n"
        for i in self.list_of_ids:
            s+= " "
            s+= i.to_string(self.module)
            s+=",\n"
        s+="} "+n +";"
        return s

    def to_enum_name(module,node):
        return yin_utils.string_to_c_formatted_name(yin_utils.node_get_identifier(node))+"_t"

    to_enum_name = staticmethod(to_enum_name)

    def get_container_name(self):
        return yin_utils.string_to_c_formatted_name(self.name)


class CPSParser:
    context = None
    module = None
    history = None
    
    imports = None
    
    key_elemts = None
    containers = None
    enum = None

    all_node_map = None
    container_map = None
    root_node = None

    def has_children(self,node):
        return node.tag in self.has_children_nodes

    def is_id_element(self,node):
        return node.tag in self.has_attr_ids

    def load_module(self,filename):
        f = search_path_for_file(filename)
        
        
    def load(self):
        et = ET.parse(self.filename)
        self.root_node = et.getroot()
        self.module = yin_ns.Module(self.filename,self.root_node)       
        self.imports = list()
        
        for i in self.root_node.findall(self.module.ns()+"import"):
            self.context['yangfiles'].load(i.get('module')+".yang")            

        self.has_children_nodes = self.module.prepend_ns_to_list(supported_list_containing_children)
        self.has_attr_ids = self.module.prepend_ns_to_list(supported_list_of_leaves_have_attr_ids)

    def __init__(self, context, filename,history_name):
        self.context = context
        self.filename = filename
        self.history = object_history.init(history_name)
        
        self.key_elemts = list()
        self.containers = list()
        self.all_node_map = {}
        self.container_map= {}
        
    def close(self):
        object_history.close(self.history)
        
    def get_current_container(self):
        return self.stack[len(self.stack)-1]

    def get_list_of_children(self, node) :
        if not node.tag in self.has_children_nodes:
            return []

        children = list()
        nodes = list(node)

        for n in nodes :
            if n.tag == self.module.ns()+'choice' :
                lst = list(n)
                for i in lst:
                    l = self.get_list_of_children(i)
                    children += l;

            else :
                children.append(n)

        return children

    def scan_for_all_nodes(self,node, path):
        if path==None:
            path = self.module.name()
        
        nodes = self.get_list_of_children(node)

        for n in nodes:
            node_name = yin_utils.node_get_identifier(n)
            if node_name == None:
                node_name = n.tag
            node_path = path+"/"+node_name
            self.all_node_map[node_path] = n

            if self.has_children(n) :
                self.scan_for_all_nodes(n)

    def get_module(self):  
        return self.module
    
    def set_module(self,module):
        self.module = module
    
    def get_node_id(self,node):
        id = yin_utils.node_get_identifier(node)
        if id == None:
            id = yin_utils.get_node_tag(self.module,node)
        return self.module.name()+":"+id
    
    
    
    def walk(self,node, path):        
        ns = self.module.name()
                
        cont_name = self.get_node_id(node)
        
        c = CPSContainer(self.module,cont_name,path,node)

        if self.container_map.get(cont_name):
            return
            #@todo need to figure out why this is necessary

        self.container_map[cont_name] = c       

        nodes = self.get_list_of_children(node)
        for n in nodes:            
            if n.tag == self.module.ns()+"uses":
                tn = self.container_map.get(n.get('name'))
                if tn != None:                    
                    nodes.remove(n);
                    for child in tn.list_of_ids:
                        child = CPSId(get_id_of_node(self.history,cont_name,child.name),                              
                              child.name,child.node,child.path,child.type)
                        c.add(child)                    
                    
        for n in nodes:                                            
            if self.is_id_element(n):
                node_name = self.get_node_id(n)
                node_ix = get_id_of_node(self.history,cont_name,node_name)
                node_path = path+"/"+node_name
                node_type = None
                if self.has_children(n):
                    node_type= CPSContainer.to_enum_name(self.module,n)
                c.add(CPSId(node_ix,c.get_container_name()+"-"+node_name,n,node_path,node_type))

            if self.has_children(n):
                self.walk(n)

        self.containers.append(c)


    def key_elements(self,node):
        leaves = yin_utils.find_child_classes_of_types(node,self.module.prepend_ns_to_list(supported_ids_at_root))

        for l in leaves:
            node_name = self.module.name()+"-"+l.get('name')
            node_id = get_id_of_node(self.history,self.module.name(),node_name)
            node_path = yin_utils.get_node_path(self.module,l, node)
            self.key_elemts.append(CPSId(node_id,node_name,l,node_path))

    def parse(self):        
        self.enum = yin_enum.EnumerationList(self.root_node,self.module,self.history)
        self.key_elements(self.root_node)
        
        self.scan_for_all_nodes(self.root_node,None)
        
        self.walk(self.root_node,self.module.name())
        

    def print_source(self):
        for i in self.containers:
            print i.to_string() + "\n"
        
        self.enum.print_source()

    def show(self):
        print "/** keys... **/ "

        for i in self.key_elemts:
            i.show()

        print "/** Containers **/"

        for i in self.containers:
            i.show()



#parser.print_source()


