
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
    "container","grouping","choice", "list", "rpc" , "case", "module","type","typedef"]


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

    all_node_map = None

    containers = None


    key_elemts = None


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
        self.containers = {}
        self.all_node_map = {}
        self.container_map= {}

    def close(self):
        object_history.close(self.history)

    def walk(self):
        self.walk_nodes(self.root_node, self.module.name())

    def walk_nodes(self, node, path):
        nodes = list(node)
        parent = path   #container path to parent

        for i in nodes:

            tag = self.module.filter_ns(i.tag)
            if i.get('name')!=None:
                n_path=path+"_"+i.get('name');
            else:
                n_path=path+"_"+tag;

            self.all_node_map[n_path] = i

            if tag == 'grouping':
                tag = 'typedef'

            if tag == 'typedef':
                self.context['types'][n_path] = i
                type = i.find(self.module.ns()+'type')
                if type!=None:
                    if type.get('name')=='enumeration':
                        self.context['enum'][n_path] = i
                    if type.get('name')=='union':
                        self.context['union'][n_path] = i
                continue

            if tag == 'choice':
                #ignore the choice itself.. and consider the cases
                for ch in list(i):
                    self.walk_nodes(ch,path)
                continue

            if tag == 'case':
                tag = 'container'

            if tag == 'enumeration':
                n_path = self.all_node_map[path]
                tag = 'container'

            if tag == 'container' or tag == 'list':
               self.containers[n_path] = i
               self.container_map[n_path] = {}
               self.walk_nodes(i,n_path)

            if tag == 'leaf' or tag == 'leaf-list' or tag=='enum':
                self.container_map[path][n_path]= i

                type = i.find(self.module.ns()+'type')
                if type!=None:
                    if type.get('name')=='enumeration':
                        self.context['enum'][n_path] = i
                        self.walk_nodes(i,n_path)

            if tag == 'uses':
                type_name = i.get('name')
                type_name.replace(':','_')
                if type_name.find(':')==-1:
                    type_name = self.module.name()+"_"+type_name

                if not type_name in self.context['types']:
                    print self.context['types'].keys()
                    print type_name
                    raise Exception ("Missing "+type_name)

                type = self.context['types'][type_name]
                type_tag = self.module.filter_ns(type.tag)
                if type_tag == 'grouping':
                    self.walk_nodes(type,path)
                    continue
                print type
                raise Exception("Invalid grouping specified ")

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


