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

import object_history
import os


def get_namespace(node):
    tag = node.tag
    lst = tag.rsplit("}")
    return lst[0] + "}"


def set_mod_name(ns, node):
    n = node.find(ns + 'prefix')
    if n is None:
        if node.tag == ns + 'module':
            return node.get('name')
    else:
        return n.get('value')
    return ""


class Module:

    def get_prefix(self, node):
        n = node.find(self.mod_ns + 'prefix')
        if n is None:
            n = node.find(self.mod_ns + 'belongs-to')
            if n is not None:
                n = n.find(self.mod_ns + 'prefix')
            if n is None:
                return ""

        return n.get('value')

    def get_module(self, node):
        if node.tag != self.mod_ns + 'module':
            node = node.find(self.mod_ns + 'module')

        if node is not None:
            return node.get('name')
        return ""

    def set_if_augments(self):
        self.augments = True

    def get_if_augments(self):
        return self.augments

    def __init__(self, filename, node):
        self.filename = filename
        self.augments = False
        self.mod_ns = get_namespace(node)	
        self.module = self.get_module(node)
        self.update_prefix(self.get_prefix(node))


    def filter_ns(self, name):
        return name[len(self.mod_ns):]

    def get_file(self):
        return os.path.basename(self.filename)

    def update_prefix(self,__prefix):
        self.prefix = __prefix
        if len(self.prefix) > 0:
            self.module_name = self.prefix
        else:
            self.module_name = self.module
        if len(self.module_name) == 0:
            raise Exception('Invalid module name/prefix')

    def ns(self):
        return self.mod_ns

    def prefix(self):
        return self.prefix

    def name(self):
        return self.module_name

    def model_name(self):
        return self.module

    def add_prefix(self,node_name):
        if node_name.find(':')==-1:
            return self.prefix + ':' + node_name
        return node_name

    def strip_prefix(self,node_name):
        loc = node_name.find(':')
        if loc!=-1:
            node_name = node_name[loc:]
        return node_name

    def replace_prefix(self,node_name):
        return self.add_prefix(self.strip_prefix(node_name))

    # Create a list that also has the NS prefix to the names
    def prepend_ns_to_list(self, types):
        l = list()
        for elem in types:
            l.append(self.ns() + elem)
        return l



