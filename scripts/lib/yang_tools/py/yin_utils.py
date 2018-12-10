#
# Copyright (c) 2018 Dell Inc.
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

# This file contains a few general purpose YIN utilities

import file_utils
import general_utils

import os
import tempfile

import sys
import subprocess

import copy

import xml.etree.ElementTree as ET

class Locator:
    def __init__(self, context, dirs_as_string=None):
        if dirs_as_string:
            self.tmpdir = dirs_as_string
        else:
            self.tmpdir = tempfile.mkdtemp()

        self.context = context
        self._loaded_nodes = {}

    def find_subdir(self,base, name):
        target = os.path.join(base,name)

        if os.path.exists(target):
            return target

        for i in os.listdir(base):
            if i==name:
                return os.path.join(base,i)
            rel = os.path.join(base,i)
            if os.path.isdir(rel):
                try:
                    rc = find_subdir(rel,name)
                    if rc!='':
                        return rc
                except:
                    pass
        return ''

   #### directory to store the OpenApi specification json files
    def mkdirOas(self):
        p = subprocess.Popen(['ar_tool.py','sysroot'],stdout=subprocess.PIPE)
        root = p.communicate()[0].strip()
        workspace = os.path.join(root,'workspace')
        sysroot = self.find_subdir(workspace, 'sysroot')
        oasdir = sysroot + '/var/www/html'
        if not os.path.exists(oasdir):
            os.makedirs(oasdir)
        return oasdir

    def get_yin_file(self, filename):
        yin_file = os.path.join(
            self.tmpdir,
            os.path.splitext(os.path.basename(filename))[0] + ".yin")
        if not os.path.exists(yin_file):
            create_yin_file(filename, yin_file)
        else:
            yang_file = search_path_for_file(filename)
            if os.path.getmtime(yang_file) > os.path.getmtime(yin_file):
                create_yin_file(filename, yin_file)

        return yin_file

    def get_openapi_file(self, filename):
        yang_file = os.path.splitext(os.path.basename(filename))[0]
        print(yang_file)
        exempted_yang_models = ['mtest','dell-support-assist','lists','ietf-netconf-acm','dell-system-common','dell-yang-types','iana-afn-safi','iana-crypt-hash','iana-if-type','ietf-inet-types','ietf-ip','ietf-routing-types','ietf-yang-types']
        if yang_file not in exempted_yang_models:
            openapi_file = os.path.join(
                self.mkdirOas(),
                os.path.splitext(os.path.basename(filename))[0] + ".json")
            if not os.path.exists(openapi_file):
                create_openapi_file(filename, openapi_file)
            return openapi_file

    def _nodes_from_yin(self,filename):
        '@type yang_file: string'
        '@rtype ET.Element'

        _file = None
        with open(filename, 'r') as f:
            _file = f.read()

        if _file.find('<module ') != -1 and _file.find('xmlns:ywx=') == -1:
            pos = _file.find('<module ') + len('<module ')
            lhs = _file[:pos] + 'xmlns:ywx="http://localhost/ignore/" '
            rhs = _file[pos:]
            _file = lhs + rhs

        try:
            return ET.fromstring(_file)
        except Exception as ex:
            pass
        return None

    def get_yin_nodes(self,filename):
        """
        Given a yin file name - load it and store in dictionary
        """
        if filename not in self._loaded_nodes:
            self._loaded_nodes[filename] = self._nodes_from_yin(filename)
        return copy.deepcopy(self._loaded_nodes[filename])


class yin_files:
    def __init__(self,context):
        self.__map = {}
        self.__context = context
        self.__nodes={}

    def __yin_name(self,yang_name):
        """
        Take a file name and return a path to a yin file.  If none exists.. create
        @yang_name just the name of a yang file (no directory expected)
        """
        yang_name = os.path.basename(yang_name)
        return self.__context.get_tmp_filename(yang_name.replace('.yang','.yin'))


    def __copy_nodes(self,yang_file):
        return copy.deepcopy(self.__nodes[yang_file])



def search_path_for_file(filename):
    path = os.getenv('YANG_MODPATH', '')
    __full_name = file_utils.search_path_for_file(filename,path)
    if __full_name!=None:
        return __full_name

    raise Exception(
        "Missing file " +
        filename +
        " please set path in YANG_MODPATH.  eg YANG_MODPATH=DIR1:DIR2")

def get_type(node):
    _type_len = len('type')
    for i in node:
        _pos = i.tag.rfind('type')
        if _pos == (len(i.tag) - _type_len):
            return i
    return None

def create_yin_file(yang_file, yin_file):
    yang_file = search_path_for_file(yang_file)
    print("Opening %s yang file and placing it in %s" %(yang_file,yin_file))
    general_utils.run_cmd(['pyang', '-o', yin_file, '-f', 'yin', yang_file])

### create OpenApi specification from yang file
def create_openapi_file(yang_file, openapi_file):
    yang_file = search_path_for_file(yang_file)
    try :
        print("Creating openapi spec %s from %s yang file " %(openapi_file,yang_file))
        general_utils.run_cmd(['pyang', '-f', 'swagger', yang_file, '-o', openapi_file])
        if os.path.isfile(openapi_file):
            with open(openapi_file, 'r') as file:
                openapi = file.read()
            openapi = openapi.replace("/\"","\"")
            with open(openapi_file, 'w') as file:
                file.write(openapi)
    except Exception as e:
        print(e)

def get_node_text(namespace, node):
    node = node.find(namespace + 'text')
    if node is not None:
        node = node.text
    if node is None:
        node = ""
    return node


def node_get_desc(module, node):
    d = node.find(module.ns() + 'description')
    if d is None:
        return ""
    return get_node_text(module.ns(), d)


def get_node_tag(module, node):
    return node.tag[len(module.ns()):]

# get the ID for a specific node.This normally is getting the 'name'
# attribute of the node


def node_get_identifier(node):
    return node.get('name')

# get the node type - if not found return "Und"


def node_get_type(module, node):
    t = node.find(module.ns() + "type")
    if t is None:
        return "Und"
    s = t.get('name')
    if s is None:
        s = "Und"
    return s

# walk through all of the children of nodes and find nodes of the type
# mentioned


def find_all_classes_of_types(nodes, list_of_types):
    lst = list()
    for i in nodes.iter():
        if i in list_of_types:
            lst.append(i)
    return lst

# find the children for the current node that are of the types mentioned


def find_child_classes_of_types(nodes, list_of_types):
    lst = list()
    for i in list(nodes):
        if i.tag in list_of_types:
            lst.append(i)
    return lst

# find the parent of the node


def find_parent(node, iter):
    par = None
    for p in node.iter():
        if iter in p:
            par = p
            break
    return par

# get the node path


def get_node_path(module, node, root_node):
    s = node_get_identifier(node)
    p = node
    while True:
        p = find_parent(p, root_node.iter())
        if p is None:
            return s
        s = node_get_identifier(p) + "/" + s

# generate an index for a node.


def get_prefix_tuple(name):
    _ix = name.find(':')
    if _ix == -1:
        return ('',name)
    return (name[:_ix],name[_ix+1:])

class IndexTracker:
    ix = 0
    begin = 0

    def __init__(self, init=0):
        self.ix = init
        self.begin = self.ix

    def get(self):
        return self.ix

    def inc(self):
        self.ix += 1

    def unused(self):
        return self.ix == self.begin
