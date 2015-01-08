
import yin_ns
import yin_utils
import sys
from sphinx.util import nodes
from DNS.Type import NULL

supported_ids_at_root = [ 
    "list","container" ]
    

supported_list_containing_children = [ 
    "container","grouping","choice", "list", "rpc" , "case", "module"]
    

supported_list_of_leaves_have_attr_ids = [ 
    "container","grouping","choice", "list", "leaf","leaf-list", "rpc", "uses" ]




def get_id_of_node(node, ix):     
    desc = node.find(yin_ns.get_ns()+'description')
    if desc != None:
        txt = desc.find(yin_ns.get_ns()+'text')
        if txt != None:
            field = txt.text;
            if field != None:
                cps_ix = field.find('cps-id=')
                if cps_ix != -1 :
                    data = field[cps_ix + len('cps-id='):]
                    just_id = data.split()
                    if len(just_id) > 0 and len(just_id[0]>0) :
                        if not ix.unused():
                            print "Invalid cps-id found in file.. please recheck"
                            sys.exit(1)
                        return int(just_id[0])         
    ix.inc()
    return ix.get()
     

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
    
    def to_string(self):        
        s = yin_utils.string_to_c_formatted_name(self.name) + " = " + str(self.cps_id)
        if self.type == None:
            self.type = yin_utils.node_get_type(self.node)
        s+= "/*!< "+self.type+" */"
        return s;
        

class CPSContainer:
    list_of_ids = list()
    name = ""
    node_path = ""
    node = None
    def __init__(self,name,path,node):
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
        desc = yin_utils.node_get_desc(self.node)
        s = ""
        if len(desc) > 0:
            s += "/* "+desc+ " */\n"
            
        s += "typedef enum {\n"
        for i in self.list_of_ids:
            s+= " "
            s+= i.to_string()
            s+=",\n"
        s+="} "+n +";"
        return s
    
    def to_enum_name(node):
        return yin_utils.string_to_c_formatted_name(yin_utils.node_get_identifier(node))+"_t"
        
    to_enum_name = staticmethod(to_enum_name)
    
    def get_container_name(self):
        return yin_utils.string_to_c_formatted_name(self.name)

class CPSParser:
    key_elemts = None
    containers = None
    
    all_node_map = None
    container_map = None
    root_node = None
    
    def has_children(self,node):
        return node.tag in self.has_children_nodes
    
    def is_id_element(self,node):
        return node.tag in self.has_attr_ids            
    
    def __init__(self):
        self.key_elemts = list()     
        self.containers = list()
        self.all_node_map = {}       
        self.container_map= {}
        self.has_children_nodes = yin_ns.prepend_ns_to_list(supported_list_containing_children)
        self.has_attr_ids = yin_ns.prepend_ns_to_list(supported_list_of_leaves_have_attr_ids)     
        
    def get_current_container(self):
        return self.stack[len(self.stack)-1]
                
    def get_list_of_children(self, node) :
        
        if not node.tag in yin_ns.prepend_ns_to_list(supported_list_containing_children):
            return [node]
        
        children = list()
        nodes = list(node)
        
        for n in nodes :
            if n.tag == yin_ns.get_ns()+'choice' :                            
                lst = list(n)
                for i in lst:                    
                    l = self.get_list_of_children(i)
                    children += l;
                    
            else :
                children.append(n)                            
        
        return children
        
    def scan_for_all_nodes(self,node):
        path = yin_utils.get_node_path(node, self.root_node)                     
        nodes = self.get_list_of_children(node)
        
        for n in nodes:
            node_name = yin_utils.node_get_identifier(n)
            node_path = path+"/"+node_name
            self.all_node_map[node_path] = n
            
            if self.has_children(n) :
                self.scan_for_all_nodes(n)                    
        
        
    def walk(self,node):
        ix = yin_utils.IndexTracker()
        path = yin_utils.get_node_path(node, self.root_node) 
        cont_name = yin_utils.node_get_identifier(node)
        c = CPSContainer(cont_name,path,node)      
           
        if self.container_map.get(cont_name):
            return
            #@todo need to figure out why this is necessary

        self.container_map[cont_name] = c;
        
        
        nodes = self.get_list_of_children(node)
        for n in nodes:
            if n.tag == yin_ns.get_ns()+"uses":
                tn = self.all_node_map.get(yin_ns.get_mod_name()+"/"+n.get('name')) 
                if tn != None:
                    n = tn
                
            if self.is_id_element(n):
                node_ix = get_id_of_node(n,ix)                
                node_name = yin_utils.node_get_identifier(n)
                node_path = path+"/"+node_name
                node_type = None
                if self.has_children(n):
                    node_type= CPSContainer.to_enum_name(n)
                c.add(CPSId(node_ix,c.get_container_name()+"-"+node_name,n,node_path,node_type))
            
            if self.has_children(n):
                self.walk(n)
            
        self.containers.append(c)
        
                
    def key_elements(self,node):
        ix = yin_utils.IndexTracker()
            
        leaves = yin_utils.find_child_classes_of_types(node,yin_ns.prepend_ns_to_list(supported_ids_at_root))
        
        for l in leaves:                
            node_id = get_id_of_node(l,ix)
            node_path = yin_utils.get_node_path(l, node)
            self.key_elemts.append(CPSId(node_id,yin_ns.get_mod_name()+"-"+l.get('name'),l,node_path))

    def parse(self, root_nodes):
        self.root_node = root_nodes
         
        yin_ns.set_mod_name(self.root_node)
        
        self.key_elements(self.root_node)
        self.scan_for_all_nodes(self.root_node)
        self.walk(self.root_node)

    def print_source(self):
        for i in self.containers:
            print i.to_string() + "\n"


    def show(self):
        print "/** keys... **/ "
        
        for i in self.key_elemts:
            i.show()
        
        print "/** Containers **/"
        
        for i in self.containers:
            i.show()


def cps_create_doc_ids(node):
    parser = CPSParser()
    parser.parse(node)
    parser.print_source()
      




