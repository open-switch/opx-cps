#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

import yin_ns
import object_history
import xml.etree.ElementTree as ET
import yin_utils


class CPSContainerElement:

    def __init__(self, name, node):
        self.name = name
        self.node = node


class CPSParser:
    supported_ids_at_root = [
        "list", "container", "rpc", 'augment']

    supported_list_containing_children = [
        'augment', "container", "grouping", "choice", "list", "rpc", "case", "module", "type", "typedef", "input", "output"]

    augment_list = []
    supported_list_of_leaves_have_attr_ids = [
        'augment', "container", "case", "list", "leaf", "leaf-list", "rpc", "choice", "input", "output"]

    __supports_duplicate_entries = ['augment']

    def __init__(self, context, model_name, filename):
        ET.register_namespace("ywx", "http://localhost/dontcare")

        self.context = context
        self.model_name = model_name
        self.__my_key = model_name
        self.filename = filename

        self.context['models'][self.__my_key] = {}

        self.key_elements = {}

        self.containers = {}
        self.all_node_map = {}
        self.container_map = {}
        self.container_keys = {}

        self.name_to_id = {}

        self.parent = {}

    def get_key(self):
        return self.__my_key

    def __ns_field(self, field_name):
        return self.context.get_ns(self.get_key()).field_name(field_name)

    def __nodes(self):
        return self.context.get_nodes(self.get_key())

    def get_key_elements(self, key, node):
        if node is None:
            raise Exception('Invalid not passed in for key %s' % key)
        if key.find('/') == -1:  # if not root node
            return key

        if key in self.key_elements:
            return self.key_elements[key]

        if key in self.container_keys:
            self.key_elements[key] = self.container_keys[key]
            return self.key_elements[key]

        _key = ""

        if key in self.parent:
            __par_name = self.parent[key]
            if __par_name not in self.key_elements:
                self.key_elements[__par_name] = \
                    self.get_key_elements(
                        __par_name,
                        self.all_node_map[__par_name])
            _key = self.key_elements[__par_name].rstrip()
            if len(_key) > 0:
                _key += ' '

        _key += key + ' '

        key_entry = node.find(self.__ns_field('key'))

        if key_entry is not None:
            for key_node in key_entry.get('value').split():
                _key_name = key + "/" + key_node
                _key += _key_name + ' '

        self.key_elements[key] = _key
        return _key

    def has_children(self, node):
        return node.tag in self.has_children_nodes

    def is_id_element(self, node):
        return node.tag in self.has_attr_ids

    def load(self, prefix=None):
        """
        Loads a specific yang file and all contained yang files.
        To perform the load of imported models, the loader class was created.
        """

        self.context.set_nodes(self.get_key(),
                               self.context.get_yang_nodes(self.filename))

        self.context.add_ns(
            self.get_key(),
            yin_ns.Module(self.filename,
                          self.__nodes()))
        self.module = self.context.get_ns(self.get_key())

        if prefix is not None:
            self.module.update_prefix(prefix)

        self.context.add_cat(self.get_key(), self.module.name())
        self.context.set_hist_name(
            self.get_key(),
            self.module.model_name() + '.yhist')

        self.imports = {}
        self.imports['module'] = list()
        self.imports['prefix'] = list()
        self.imports['map'] = {}

        for i in self.__nodes().findall(self.__ns_field("import")):
            prefix = i.find(self.__ns_field('prefix'))
            if prefix is not None:
                prefix = prefix.get('value')
            if prefix is not None:
                self.imports['prefix'].append(prefix)

            _import_name = i.get('module') + ".yang"
            _import_prefix = prefix

            print(
                "Loading module %s with prefix %s" %
                (_import_name, _import_prefix))

            self.context['loader'].load(_import_name, prefix=prefix)
            self.imports['module'].append(i.get('module'))
            self.imports['map'][_import_name] = _import_prefix

        self.has_children_nodes = self.context.get_ns(self.get_key()).prepend_ns_to_list(
            self.supported_list_containing_children)

        self.has_attr_ids = self.context.get_ns(self.get_key()).prepend_ns_to_list(
            self.supported_list_of_leaves_have_attr_ids)

    def close(self):
        object_history.close(self.history)

    def walk(self):
        if self.module.name() in self.container_map:
            return

        self.container_map[self.module.name()] = list()
        self.all_node_map[self.module.name()] = self.__nodes()
        self.container_keys[self.module.name()] = self.module.name() + " "

        self.pre_parse_augments(self.__nodes(), self.module.name() + ':')

        print "Creating type mapping..."
        self.parse_types(self.__nodes(), self.module.name() + ':')

        print "Updating prefix (%s) related mapping" % self.module.name()
        self.update_node_prefixes(self.__nodes())

        print "Parsing Yang Model %s" % (self.module.name())
        self.walk_nodes(self.__nodes(), self.module.name())

        self.handle_augments(self.__nodes(), self.module.name() + ':')

        self.handle_keys()
        self.fix_enums()
        print "Yang parsing complete"

    def add_module_fb_mapping(self, container, key):
        """ add the entry in the container to both prefix/ and prefix: based  """
        _prefix_a = self.module.name() + "/"
        _prefix_b = self.module.name() + ":"
        if key.find(_prefix_a) != 0:
            return

        _alias = _prefix_b + key[len(_prefix_a):]
        container[_alias] = container[key]

    def fix_enums(self):
        for i in self.context['enum'].keys():
            self.add_module_fb_mapping(self.context['enum'], i)
        for i in self.context['types'].keys():
            self.add_module_fb_mapping(self.context['types'], i)
        for i in self.context['union'].keys():
            self.add_module_fb_mapping(self.context['types'], i)

    def update_node_prefixes(self, node):
        """
        Update node types to use local to the file prefixes if none specified
        """
        for i in node.iter():
            tag = self.module.filter_ns(i.tag)

            _name = i.get('name')
            if _name is None:
                continue

            _prefix_and_name = self.module.add_prefix(_name)

            if tag == 'uses':
                i.set('name', _prefix_and_name)

            if tag == 'type':
                # use the names that have been discovered already - otherwise
                # could be a base type.
                if self.context.name_is_a_type(_prefix_and_name):
                    i.set('name', _prefix_and_name)

    def stamp_augmented_children(self, parent, ns):
        for i in list(parent):
            self.stamp_augmented_children(i, ns)
            i.set('augmented', True)
            i.set('target-namespace', ns)

    def _generate_augment_key(self, augment_key):
        """
        Input is in form of /if:interfaces/if:interface/ip:ipv4/ip:address and result should be
                            ip/if/interfaces/interface/ipv4/address

                            Or...
                            /if:interfaces/if:interface/ip:ipv4/ip:address/ns:name
                            ns/ip/if/interfaces/interface/ipv4/address/name
        """
        _lst = augment_key.split('/')
        if _lst[0] == '':
            _lst = _lst[1:]
        _result = ''

        _last_ns = ''
        _last_entry_list = []

        _ns_lst = []
        _ns_entries = []

        for i in range(0, len(_lst)):
            _comp = yin_utils.get_prefix_tuple(_lst[i])

            if _comp[0] != _last_ns:
                _last_entry_list = []
                _ns_entries.append(_last_entry_list)

                _last_ns = _comp[0]
                _ns_lst.append(_last_ns)

            _last_entry_list.append(_comp[1])

        _result = ''

        def _add_slash(list_of_items):
            return '/'.join(list_of_items)

        for i in range(0, len(_ns_entries)):
            if len(_result) > 0 and _result[0] != '/':
                _result = '/' + _result
            _result = _ns_lst[i] + _result

            for ent in _ns_entries[i]:
                _result += '/' + ent

        if _result[0] == '/':
            _result = _result[1:]
        return _result

    def pre_parse_augments(self, parent, path):
        for i in parent:
            tag = self.module.filter_ns(i.tag)

            # if type is augment.. then set the items 'name' to the augmented
            # class
            if tag == 'augment':
                _tgt_node = i.get('target-node')

                _cps_name = self._generate_augment_key(_tgt_node)
                _prefix = _cps_name.split('/', 1)[0]

                if _cps_name.find('/') == -1:
                    _ns = self.context.get_ns(self.get_key()).prefix()
                else:
                    _ns = _prefix

                i.set('target-namespace', _ns)
                i.set('name', _cps_name)
                i.set('augmented', True)
                self. stamp_augmented_children(i, _ns)

                _tgt_node = _cps_name
                _ns = i.get('target-namespace')

                if _ns == self.context.get_ns(self.get_key()).prefix():
                    _key_model = self
                    _key_path = _key_model.get_key_elements(
                        _tgt_node, i.get('augment'))
                    _augmented_node = _key_model.all_node_map[
                        _ns + '/' + _tgt_node]
                else:
                    _key_model = self.context['loader'].yin_map[
                        self.context['model-names'][_ns]]
                    _key_path = _key_model.get_key_elements(_tgt_node, i)
                    _key_path = self.module.name() + ' ' + _key_path
                    if _tgt_node not in _key_model.all_node_map:
                        raise Exception(
                            'Missing key mapping for augment node %s' %
                            _tgt_node)
                    _augmented_node = _key_model.all_node_map[_tgt_node]

                i.set('key-path', _key_path)
                i.set('augment', _augmented_node)

    def handle_augments(self, parent, path):
        for i in parent:
            tag = self.module.filter_ns(i.tag)

            # if type is augment.. then set the items 'name' to the augmented
            # class
            if tag == 'augment':
                _tgt_node = i.get('name')
                _ns = i.get('target-namespace')

                if _ns == self.context.get_ns(self.get_key()).prefix():
                    _key_model = self
                    _key_path = _key_model.get_key_elements(
                        _tgt_node, i.get('augment'))
                    _augmented_node = _key_model.all_node_map[
                        _ns + '/' + _tgt_node]
                else:
                    _key_model = self.context['loader'].yin_map[
                        self.context['model-names'][_ns]]
                    _key_path = _key_model.get_key_elements(_tgt_node, i)
                    _key_path = self.module.name() + ' ' + _key_path
                    if _tgt_node not in _key_model.all_node_map:
                        raise Exception(
                            'Missing key mapping for augment node %s' %
                            _tgt_node)
                    _augmented_node = _key_model.all_node_map[_tgt_node]

                self.module.set_if_augments()
                if _key_model not in self.augment_list:
                    self.augment_list.append(_key_model)
                i.set('augment', _augmented_node)
                i.set('key-path', _key_path)

    def parse_types(self, parent, path):
        """
            Search through all children starting at parent (recursively) for types.
            The "path" variable denotes the path to the parent node in text.

            eg.. self.parse_types(root_node,'/somewhere')
        """
        valid_types = ['typedef', 'identity']
            #two nodes that can be part of types

        for i in parent:
            tag = self.module.filter_ns(i.tag)

            id = tag
            if tag == 'config':
                parent.set('read-only', True)
                continue
            if tag == 'must':
                continue

            if i.get('name') is not None:
                id = i.get('name')

            full_name = path
            # if "entry:" then skip the / eg.. entry:/ = bad
            if full_name[len(full_name) - 1] != ':':
                full_name += "/"
            full_name += id
            type_name = self.module.name() + ':' + id

            # recurse children
            if len(i) > 0:
                self.parse_types(i, full_name)

            if tag == 'grouping':
                if type_name in self.context['grouping']:
                    print(
                        'Discovered multiple references to %s - ignoring' %
                        type_name)
                    continue
                self.context['grouping'][type_name] = i
                continue

            if tag == 'leaf' or tag == 'leaf-list':
                type = i.find(self.module.ns() + 'type')
                if type.get('name') == 'enumeration':
                    tag = 'typedef'

            if tag == 'typedef':
                type = i.find(self.module.ns() + 'type')
                if type is not None:
                    if type.get('name') == 'enumeration':
                        self.context['enum'][full_name] = i
                        continue
                    if type.get('name') == 'union':
                        self.context['union'][full_name] = i
                        continue

            if tag in valid_types:
                if type_name in self.context['types']:
                    continue

                self.context['types'][type_name] = i

            if tag == 'identity':
                _base = i.find(self.module.ns() + 'base')
                _identity_ = ""

                # resolve the base to an attribute called _identity_ including
                # base idents
                if _base is not None:
                    _base = self.module.add_prefix(_base.get('name'))

                    if not _base in self.context['types']:
                        raise Exception('Failed to locate type required.')

                    _base_node = self.context['types'][_base]

                    _identity_ = _base_node.get('__identity__')
                    if _base_node.get('__children__') is not None:
                        _base_node.set(
                            '__children__',
                            _base_node.get(
                                '__children__') +
                            ' ' +
                            type_name)
                    else:
                        _base_node.set('__children__', type_name)

                    if _identity_ is None:
                        _identity_ = ''

                    if len(_identity_) > 0:
                        _identity_ += '/'

                _identity_ += type_name

                i.set('__identity__', _identity_)

    def is_augmented(self, path):
        parent_path = "/".join(path.split("/")[:-1])
        if parent_path in self.all_node_map:
            if self.module.filter_ns(self.all_node_map[parent_path].tag) == "augment":
                return (True, self.all_node_map[parent_path])
            else:
                return self.is_augmented(parent_path)
        else:
            return (False, None)

    __tags_to_ingore_walk_nodes = [
        'config',
        'key',
        'must',
        'description',
        'mandatory',
        'max-elements',
        'min-elements',
        'range',
        'value',
        'when',
        'default',
        'feature']

    def walk_nodes(self, node, path):
        nodes = list(node)
        parent = path  # container path to parent

        parent_tag = self.module.filter_ns(self.all_node_map[parent].tag)

        for i in nodes:
            tag = self.module.filter_ns(i.tag)

            if tag in self.__tags_to_ingore_walk_nodes:
                continue

            if i.get('name') is not None:
                n_path = path + "/" + i.get('name')
            else:
                n_path = path + "/" + tag

            # can have repeated nodes for some classes (augment)
            if tag not in self.__supports_duplicate_entries:
                if n_path in self.all_node_map:
                    # Handle the case of leaf named description and description
                    # element already existing at this level
                    if (self.all_node_map[n_path].tag.find('description')) and (i.get('name') == 'description'):
                        print "description leaf and description tag found"
                    else:
                        continue

            # fill in parent
            self.parent[n_path] = path

            if tag == 'grouping' or tag == 'typedef':
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

            # make a copy of the current node as node can be contained as part of grouping in
            # augmented container or non-augmented container. so when marking augmented container
            # leaves we don't mark the leaves as augmented for the container which is not augmenting
            # and still using the grouping object
            self.all_node_map[n_path] = i.copy()

            if tag == 'choice' or tag == 'input' or tag == 'output' or tag == 'rpc':
                tag = 'container'

            if tag == 'case':
                tag = 'container'

            if tag == 'enumeration':
                n_path = self.all_node_map[path]
                tag = 'container'

            if tag == 'container' or tag == 'list' or tag == 'rpc' or tag == 'augment':

                __add_ = True

                # in the case of dup entries possible don't add if already a
                # node exists
                if tag in self.__supports_duplicate_entries:
                    if n_path in self.containers:
                        __add_ = False

                if __add_:
                    self.containers[n_path] = i
                    self.container_map[n_path] = list()
                else:
                    # add the children of this node the previous node
                    self.containers[n_path].extend(list(i))

                # if the augment is there, then append and add a key path if
                # necessary
                if __add_:
                    self.container_map[path].append(
                        CPSContainerElement(n_path, i))

                    _key_node = i
                    _key_prefix = n_path
                    _key_model = self

                    _key_path = i.get('key-path')

                    if _key_path is None:
                        _key_path = _key_model.get_key_elements(
                            _key_prefix, _key_node)

                    self.container_keys[n_path] = _key_path

                self.walk_nodes(i, n_path)

            if tag == 'leaf' or tag == 'leaf-list' or tag == 'enum':
                self.container_map[path].append(CPSContainerElement(n_path, i))

                _type = i.find(self.module.ns() + 'type')
                if _type is not None:
                    if _type.get('name') == 'enumeration':
                        self.context['enum'][n_path] = i
                        self.walk_nodes(i, n_path)

            if tag == 'uses':
                type_name = i.get('name')

                if type_name.find(':') == -1:
                    raise Exception(
                        "Missing _type name... should already be specified")

                if not type_name in self.context['grouping']:
                    print self.context['grouping'].keys()
                    print type_name
                    raise Exception("Missing " + type_name)

                _type = self.context['grouping'][type_name]
                (ret_val, ret_node) = self.is_augmented(path)
                if ret_val:
                    self.stamp_augmented_children(
                        _type,
                        ret_node.get('target-namespace'))
                type_tag = self.module.filter_ns(_type.tag)
                if type_tag == 'grouping':
                    self.walk_nodes(_type, path)
                    continue
                print _type
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
            if self.all_node_map[k].tag not in self.has_attr_ids:
                continue
            if k not in self.key_elements:
                self.key_elements[k] = self.get_key_elements(
                    k, self.all_node_map[k])

        self.keys = self.key_elements
