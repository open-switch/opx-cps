
import yin_ns
import yin_utils
import sys
import object_history
import os

import xml.etree.ElementTree as ET

import tempfile

supported_ids_at_root = [
    "list","container" ]

supported_list_containing_children = [
    "container","grouping","choice", "list", "rpc" , "case", "module","type","typedef"]


supported_list_of_leaves_have_attr_ids = [
    "container","grouping","choice", "list", "leaf","leaf-list", "rpc", "uses" ]

class CPSContainerElement:
    name = None
    node = None
    def __init__(self,name,node):
        self.name = name
        self.node = node

class CPSParser:
    context = None
    module = None
    history = None

    all_node_map = None

    containers = None

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
        self.container_map[self.module.name()] = list()
        self.all_node_map[self.module.name()] = self.root_node
        self.walk_nodes(self.root_node, self.module.name())

    def walk_nodes(self, node, path):
        nodes = list(node)
        parent = path   #container path to parent

        for i in node.iter():
            tag = self.module.filter_ns(i.tag)
            if tag == 'uses':
                n = i.get('name')
                if n.find(':')==-1:
                    n = self.module.name()+':'+n
                i.set('name',n)

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
               self.container_map[n_path] = list()
               self.container_map[path].append(CPSContainerElement(n_path,i))
               self.walk_nodes(i,n_path)

            if tag == 'leaf' or tag == 'leaf-list' or tag=='enum':
                self.container_map[path].append(CPSContainerElement(n_path,i))

                type = i.find(self.module.ns()+'type')
                if type!=None:
                    if type.get('name')=='enumeration':
                        self.context['enum'][n_path] = i
                        self.walk_nodes(i,n_path)

            if tag == 'uses':
                type_name = i.get('name')
                if type_name.find(':')==-1:
                    type_name = self.module.name()+"_"+type_name
                type_name = type_name.replace(':','_')

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


