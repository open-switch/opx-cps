
import yin_ns
import yin_utils
import sys
import object_history
import os

import xml.etree.ElementTree as ET

import tempfile

supported_ids_at_root = [
    "list","container","rpc" ]

supported_list_containing_children = [
    "container","grouping","choice", "list", "rpc" , "case", "module","type","typedef","input","output"]

supported_list_of_leaves_have_attr_ids = [
    "container","case", "list", "leaf","leaf-list", "rpc", "choice","input","output" ]

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

    def load(self,prefix=None):

        et = ET.parse(self.filename)
        self.root_node = et.getroot()
        self.module = yin_ns.Module(self.filename,self.root_node)
        if prefix!=None:
                self.module.module_name = prefix

        self.imports = {}
        self.imports['module'] = list()
        self.imports['prefix'] = list()

        for i in self.root_node.findall(self.module.ns()+"import"):
            prefix = i.find(self.module.ns()+'prefix')
            if prefix!=None:
                prefix = prefix.get('value')
            if prefix!=None:
                self.imports['prefix'].append(prefix)
            print "Loading module with prefix %s" % prefix
            self.context['loader'].load(i.get('module')+".yang",prefix=prefix)
            self.imports['module'].append(i.get('module'))

        self.has_children_nodes = self.module.prepend_ns_to_list(supported_list_containing_children)
        self.has_attr_ids = self.module.prepend_ns_to_list(supported_list_of_leaves_have_attr_ids)


    def __init__(self, context, filename):
        self.context = context
        self.filename = filename

        self.key_elemts = list()
        self.containers = {}
        self.all_node_map = {}
        self.container_map= {}
        self.container_keys={}
        self.name_to_id={}
        self.parent={}

    def close(self):
        object_history.close(self.history)

    def walk(self):
        if self.module.name() in self.container_map:
            return
        self.container_map[self.module.name()] = list()
        self.all_node_map[self.module.name()] = self.root_node
        self.container_keys[self.module.name()] = self.module.name()+ " "
        self.parse_types(self.root_node)
        print self.context['types']
        self.fix_namespace(self.root_node)
        self.walk_nodes(self.root_node, self.module.name())
        self.handle_keys()
        self.fix_enums()

    def path_to_prefix(self,dict,key):
        if key.find(self.module.name()+"/")==0:
            node = key.replace(self.module.name()+"/",self.module.name()+":",1)
            dict[node] = dict[key]

    def fix_enums(self):
        for i in self.context['enum'].keys():
            self.path_to_prefix(self.context['enum'],i)
        for i in self.context['types'].keys():
            self.path_to_prefix(self.context['types'],i)


    def fix_namespace(self,node):
        for i in node.iter():
            tag = self.module.filter_ns(i.tag)
            n = None
            if tag == 'uses':
                n = i.get('name')

            if tag == 'type':
                name = i.get('name')
                if name!=None and self.module.name()+':'+name in self.context['types']:
                    n = name

            if n!=None:
                if n.find(':')==-1:
                    n = self.module.name()+':'+n
                i.set('name',n)

    def parse_types(self,parent):
        for i in parent:
            if len(i) > 0:
                self.parse_types(i)

            tag = self.module.filter_ns(i.tag)

            id = self.module.name()+':'+tag
            if i.get('name')!=None:
                id = self.module.name()+':'+i.get('name')

            if tag == 'grouping':
                tag = 'typedef'

            if tag == 'leaf' or tag == 'leaf-list':
                type = i.find(self.module.ns()+'type')
                if type.get('name')=='enumeration':
                    tag = 'typedef'

            if tag == 'typedef':
                if id in self.context['types']:
                    continue
                    #raise Exception("Duplicate entry in type name database..."+id)

                self.context['types'][id] = i
                type = i.find(self.module.ns()+'type')
                if type!=None:
                    if type.get('name')=='enumeration':
                        self.context['enum'][id] = i
                    if type.get('name')=='union':
                        self.context['union'][id] = i
                continue

    def walk_nodes(self, node, path):
        nodes = list(node)
        parent = path   #container path to parent

        parent_tag = self.module.filter_ns(self.all_node_map[parent].tag)

        for i in nodes:
            tag = self.module.filter_ns(i.tag)

            if i.get('name')!=None:
                n_path=path+"/"+i.get('name');
            else:
                n_path=path+"/"+tag;

            id = self.module.name()+':'+tag
            if i.get('name')!=None:
                id = self.module.name()+':'+i.get('name')

            if n_path in  self.all_node_map:
                continue

            self.parent[n_path]=path

            if tag == 'grouping':
                tag = 'typedef'

            if tag == 'typedef':
                continue

            #in the case tht the parent tag is a choice and you parsing a non-case... then add a case for the standard
            # As a shorthand, the "case" statement can be omitted if the branch contains a single "anyxml", "container",
            # "leaf", "list", or "leaf-list" statement.  In this case, the identifier of the case node is the same as
            # the identifier in the branch statement.
            if parent_tag == 'choice' and (tag == 'anyxml' or tag == 'container' or tag == 'leaf' or tag =='list' or tag == 'leaf-list'):
                new_node = ET.Element(self.module.ns()+'case',attrib={'name':i.get('name')})
                new_node.append(i)
                i = new_node
                tag = 'case'

            self.all_node_map[n_path] = i

            if tag == 'choice' or tag == 'input' or tag=='output' or tag=='rpc':
                tag = 'container'

            if tag == 'case':
                tag = 'container'

            if tag == 'enumeration':
                n_path = self.all_node_map[path]
                tag = 'container'

            if tag == 'container' or tag == 'list' or tag == 'rpc':
               self.containers[n_path] = i
               if n_path not in self.container_map:
                   self.container_map[n_path] = list()
               self.container_map[path].append(CPSContainerElement(n_path,i))
               self.container_keys[n_path] = self.container_keys[path]
               self.container_keys[n_path] += n_path +" "
               key_entry = i.find(self.module.ns()+'key')
               if key_entry != None:
                    for key_node in key_entry.get('value').split():
                        self.container_keys[n_path] += n_path+"/"+key_node+" "

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
                    raise Exception("Missing type name... should already be specified")

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


    def handle_keys(self):
        self.elem_with_keys = {}
        for i in self.container_keys.keys():
            node = self.all_node_map[i]
            if node.find(self.module.ns()+'key')== None:
                continue
            self.elem_with_keys[i] = self.container_keys[i]

        self.keys = {}
        for k in self.all_node_map:
            if not self.is_id_element(self.all_node_map[k]) : continue
            self.fill_element_key(k)

    def fill_element_key(self,key):
        key_base = ""

        if key.find('/')!=-1:
            if key in self.container_keys:
                key_base = self.container_keys[key]
            else:
                parent = self.parent[key]
                if not parent in self.keys:
                    self.fill_element_key(parent)
                key_base += self.keys[parent]
                key_base += " "
                key_base += key
        else:
            keys_base = self.container_keys[key]

        self.keys[key] = key_base

