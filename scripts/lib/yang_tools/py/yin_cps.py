#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN #AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
#  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

import yin_ns
import yin_utils
import sys
import object_history
import os

import xml.etree.ElementTree as ET

import tempfile


class CPSContainerElement:

    def __init__(self, name, node):
        self.name = name
        self.node = node


class CPSParser:
    supported_ids_at_root = [
        "list", "container", "rpc", 'augment']

    supported_list_containing_children = [
        'augment',"container", "grouping", "choice", "list", "rpc", "case", "module", "type", "typedef", "input", "output"]

    augment_list = []
    supported_list_of_leaves_have_attr_ids = [
        'augment',"container", "case", "list", "leaf", "leaf-list", "rpc", "choice", "input", "output"]

    __supports_duplicate_entries= ['augment']

    def __init__(self, context, filename):
        ET.register_namespace("ywx", "http://localhost/dontcare")

        self.context = context
        self.filename = filename

        self.key_elements = {}

        self.containers = {}
        self.all_node_map = {}
        self.container_map = {}
        self.container_keys = {}

        self.name_to_id = {}

        self.parent = {}

    def get_key_elements(self,key,node):

        if key.find('/') == -1:    #if not root node
            return key

        if key in self.key_elements:
            return self.key_elements[key]

        if key in self.container_keys:
            self.key_elements[key] = self.container_keys[key]
            return self.key_elements[key]

        __key = ""

        if key in self.parent:
             __par_name = self.parent[key]
             if __par_name not in self.key_elements:
                self.key_elements[__par_name] = \
                    self.get_key_elements(__par_name,self.all_node_map[__par_name])
             __key = self.key_elements[__par_name].rstrip()
             if len(__key) > 0: __key+=' '

        __key += key + ' '

        key_entry = node.find(self.module.ns() + 'key')

        if key_entry is not None:
            for key_node in key_entry.get('value').split():
                _key_name = key + "/" + key_node
                __key += _key_name + ' '

        self.key_elements[key] = __key
        return __key

    def has_children(self, node):
        return node.tag in self.has_children_nodes

    def is_id_element(self, node):
        return node.tag in self.has_attr_ids

    def load(self, prefix=None):
        """
        Loads a specific yang file and all contained yang files.
        To perform the load of imported models, the loader class was created.
        """
        _file = ''
        with open(self.filename, 'r') as f:
            _file = f.read()

        if _file.find('<module ') != -1 and _file.find('xmlns:ywx=') == -1:
            pos = _file.find('<module ') + len('<module ')
            lhs = _file[:pos] + 'xmlns:ywx="http://localhost/ignore/" '
            rhs = _file[pos:]
            _file = lhs + rhs

        try:
            self.root_node = ET.fromstring(_file)
        except Exception as ex:
            print "Failed to process ", self.filename
            print ex
            sys.exit(1)

        self.module = yin_ns.Module(self.filename, self.root_node)

        if prefix is not None:
            self.module.update_prefix(prefix)

        self.imports = {}
        self.imports['module'] = list()
        self.imports['prefix'] = list()

        for i in self.root_node.findall(self.module.ns() + "import"):
            prefix = i.find(self.module.ns() + 'prefix')
            if prefix is not None:
                prefix = prefix.get('value')
            if prefix is not None:
                self.imports['prefix'].append(prefix)
            print "Loading module with prefix %s" % prefix
            self.context['file-finder'].load(
                i.get('module') + ".yang",
                prefix=prefix)
            self.imports['module'].append(i.get('module'))

        self.has_children_nodes = self.module.prepend_ns_to_list(
            self.supported_list_containing_children)
        self.has_attr_ids = self.module.prepend_ns_to_list(
            self.supported_list_of_leaves_have_attr_ids)


    def close(self):
        object_history.close(self.history)

    def walk(self):
        if self.module.name() in self.container_map:
            return

        self.container_map[self.module.name()] = list()
        self.all_node_map[self.module.name()] = self.root_node
        self.container_keys[self.module.name()] = self.module.name() + " "
        self.handle_augments(self.root_node, self.module.name() + ':')

        print "Creating type mapping..."
        self.parse_types(self.root_node, self.module.name() + ':')

        print "Updating prefix (%s) related mapping" % self.module.name()
        self.update_node_prefixes(self.root_node)

        print "Parsing Yang Model %s" % (self.module.name())
        self.walk_nodes(self.root_node, self.module.name())
        self.handle_keys()
        self.fix_enums()
        print "Yang parsing complete"

    def add_module_fb_mapping(self, dict, key):
        """ add the entry in the dict to both prefix/ and prefix: based  """
        _prefix_a = self.module.name() + "/"
        _prefix_b = self.module.name() + ":"
        if key.find(_prefix_a) != 0:
            return

        _alias =  _prefix_b+key[len(_prefix_a):]
        dict[_alias] = dict[key]

    def fix_enums(self):
        for i in self.context['enum'].keys():
            self.add_module_fb_mapping(self.context['enum'], i)
        for i in self.context['types'].keys():
            self.add_module_fb_mapping(self.context['types'], i)

    def update_node_prefixes(self, node):
        """
        Update node types to use local to the file prefixes if none specified
        """
        for i in node.iter():
            tag = self.module.filter_ns(i.tag)
            n = None

            if tag == 'uses':
                n = i.get('name')

            if tag == 'type':
                name = i.get('name')
                #make sure it is a type we want
                if name is not None and self.module.name() + ':' + name in self.context['types']:
                    n = name

            if n is not None: # any yypes refer to local types by default
                n = self.module.add_prefix(n)
                i.set('name', n)

    def stamp_augmented_children(self, parent, ns):
        lst = list()
        for i in list(parent):
            self.stamp_augmented_children(i, ns)
            i.set('augmented', True)
            i.set('target-namespace',ns)


    def handle_augments(self,parent,path):
        for i in parent:
            tag = self.module.filter_ns(i.tag)

            #if type is augment.. then set the items 'name' to the augmented class
            if tag == 'augment':
                _tgt_node = i.get('target-node')

                if _tgt_node[:1] == '/':
                    _tgt_node = _tgt_node[1:]

                if _tgt_node.find(':')==-1:
                    print("Failed to find prefix in augment for %s." % _tgt_node )
                    __ns = self.module.ns
                else:
                    __ns = _tgt_node[:_tgt_node.find(':')]

                _tgt_node = _tgt_node.replace(__ns+':','')
                _tgt_node = __ns +"/" +_tgt_node
                _key_model = self.context['loader'].yin_map[self.context['model-names'][__ns]]
                __key_path =  _key_model.get_key_elements(_tgt_node,i.get('augment'))
                __key_path =  self.module.name()+ ' ' +__key_path
                __augmented_node = _key_model.all_node_map[_tgt_node]

                self.module.set_if_augments()
                if _key_model not in self.augment_list:
                    self.augment_list.append(_key_model)
                i.set('target-namespace',__ns)
                i.set('name',_tgt_node)
                i.set('model',_key_model)
                i.set('augment',__augmented_node)
                i.set('key-path',__key_path)
                i.set('augmented', True)
                self. stamp_augmented_children(i, __ns)


    def parse_types(self, parent, path):
        """
            Search through all children starting at parent (recursively) for types.
            The "path" variable denotes the path to the parent node in text.

            eg.. self.parse_types(root_node,'/somewhere')
        """
        valid_types = ['typedef','identity' ] #two nodes that can be part of types

        for i in parent:
            tag = self.module.filter_ns(i.tag)

            id = tag

            if i.get('name') is not None:
                id = i.get('name')

            full_name = path
            #if "entry:" then skip the / eg.. entry:/ = bad
            if full_name[len(full_name) - 1] != ':':
                full_name += "/"
            full_name += id
            type_name = self.module.name() + ':' + id

            #recurse children
            if len(i) > 0:
                self.parse_types(i, full_name)

            if tag == 'grouping':
                tag = 'typedef'

            if tag == 'leaf' or tag == 'leaf-list':
                type = i.find(self.module.ns() + 'type')
                if type.get('name') == 'enumeration':
                    tag = 'typedef'

            if tag in valid_types:
                if type_name in self.context['types']:
                    continue
                    # raise Exception("Duplicate entry in type name
                    # database..."+id)

                self.context['types'][type_name] = i

            if tag == 'typedef':
                type = i.find(self.module.ns() + 'type')
                if type is not None:
                    if type.get('name') == 'enumeration':
                        self.context['enum'][full_name] = i
                    if type.get('name') == 'union':
                        self.context['union'][full_name] = i
                continue

            if tag == 'identity':
                _base = i.find(self.module.ns()+'base')
                _identity_ = ""

                #resolve the base to an attribute called _identity_ including base idents
                if _base != None:
                    _base = self.module.add_prefix(_base.get('name'))

                    if not _base in self.context['types']:
                        raise Exception('Failed to locate type required.')

                    _base_node = self.context['types'][_base]

                    _identity_ = _base_node.get('__identity__')
                    if  _base_node.get('__children__')!=None:
                        _base_node.set('__children__',_base_node.get('__children__')+' '+type_name)
                    else:
                        _base_node.set('__children__',type_name)

                    if _identity_ == None: _identity_ = ''

                    if len(_identity_)>0:
                        _identity_+='/'

                _identity_+=type_name

                i.set('__identity__',_identity_)


    def walk_nodes(self, node, path):
        nodes = list(node)
        parent = path  # container path to parent

        parent_tag = self.module.filter_ns(self.all_node_map[parent].tag)

        for i in nodes:
            tag = self.module.filter_ns(i.tag)

            if i.get('name') is not None:
                n_path = path + "/" + i.get('name')
            else:
                n_path = path + "/" + tag

            id = self.module.name() + ':' + tag

            if i.get('name') is not None:
                id = self.module.name() + ':' + i.get('name')

            #can have repeated nodes for some classes (augment)
            if tag not in self.__supports_duplicate_entries:
                if n_path in self.all_node_map:
                    continue

            #fill in parent
            self.parent[n_path] = path

            if tag == 'grouping':
                tag = 'typedef'

            if tag == 'typedef':
                continue

            # in the case tht the parent tag is a choice and you parsing a non-case... then add a case for the standard
            # As a shorthand, the "case" statement can be omitted if the branch contains a single "anyxml", "container",
            # "leaf", "list", or "leaf-list" statement.  In this case, the identifier of the case node is the same as
            # the identifier in the branch statement.
            if parent_tag == 'choice' and (tag == 'anyxml' or tag == 'container' or tag == 'leaf' or tag == 'list' or tag == 'leaf-list'):
                new_node = ET.Element(
                    self.module.ns() + 'case',
                    attrib={'name': i.get('name')})
                new_node.append(i)
                i = new_node
                tag = 'case'

            self.all_node_map[n_path] = i

            if tag == 'choice' or tag == 'input' or tag == 'output' or tag == 'rpc':
                tag = 'container'

            if tag == 'case':
                tag = 'container'

            if tag == 'enumeration':
                n_path = self.all_node_map[path]
                tag = 'container'

            if tag == 'container' or tag == 'list' or tag == 'rpc' or tag=='augment':

                __add_ = True

                #in the case of dup entries possible don't add if already a node exists
                if tag in self.__supports_duplicate_entries:
                    if n_path in self.containers:
                        __add_ = False

                if __add_==True:
                    self.containers[n_path] = i
                    self.container_map[n_path] = list()
                else:
                    #add the children of this node the previous node
                    self.containers[n_path].extend(list(i))

                #if the augment is there, then append and add a key path if necessary
                if __add_==True:
                    self.container_map[path].append(CPSContainerElement(n_path, i))

                    _key_node = i
                    _key_prefix = n_path
                    _key_model = self

                    _key_path = i.get('key-path')

                    if _key_path==None:
                        _key_path = _key_model.get_key_elements(_key_prefix,_key_node)

                    self.container_keys[n_path] = _key_path

                self.walk_nodes(i, n_path)

            if tag == 'leaf' or tag == 'leaf-list' or tag == 'enum':
                self.container_map[path].append(CPSContainerElement(n_path, i))

                type = i.find(self.module.ns() + 'type')
                if type is not None:
                    if type.get('name') == 'enumeration':
                        self.context['enum'][n_path] = i
                        self.walk_nodes(i, n_path)

            if tag == 'uses':
                type_name = i.get('name')
                if type_name.find(':') == -1:
                    raise Exception(
                        "Missing type name... should already be specified")

                if not type_name in self.context['types']:
                    print self.context['types'].keys()
                    print type_name
                    raise Exception("Missing " + type_name)

                type = self.context['types'][type_name]
                type_tag = self.module.filter_ns(type.tag)
                if type_tag == 'grouping':
                    self.walk_nodes(type, path)
                    continue
                print type
                raise Exception("Invalid grouping specified ")

    def handle_keys(self):
        self.elem_with_keys = {}
        for i in self.container_keys.keys():
            node = self.all_node_map[i]
            if node.find(self.module.ns() + 'key') is None:
                continue
            self.elem_with_keys[i] = self.container_keys[i]

        self.keys = {}
        for k in self.all_node_map:
            if self.all_node_map[k].tag not in self.has_attr_ids: continue
            if k not in self.key_elements:
                self.key_elements[k] = self.get_key_elements(k,self.all_node_map[k])

        self.keys = self.key_elements

