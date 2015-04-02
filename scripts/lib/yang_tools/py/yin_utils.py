# This file contains a few general purpose YIN utilities
import os
import subprocess

def run_cmd(args):
    p = subprocess.Popen(args,stdout=subprocess.PIPE)
    stdout = p.communicate()[0]
    if p.wait()!=0:
        print(stdout)


def search_path_for_file(filename):
    path = os.getenv('YANG_PATH','')
    for i in path.split(':'):
        f = os.path.join(i,filename)
        if os.path.exists(f):
            return f
    raise Exception("Missing file "+filename+" please set path in YANG_PATH.  eg YANG_PATH=DIR1:DIR2")

def get_yang_history_file_name(filename):
    yang_hist_name = os.path.splitext(os.path.basename(filename))[0]+".yhist"
    default_dir = os.path.dirname(filename)

    try:
        return search_path_for_file(yang_hist_name)
    except:
        name = ""
    name = os.path.join(default_dir,yang_hist_name)

    if os.path.exists(name):
        return name

    path = os.getenv('YANG_PATH','')
    for i in path.split(':'):
        if 'yanghist' in i:
            return i

    yang_file = search_path_for_file(filename)
    return os.path.join(os.path.dirname(yang_file),yang_hist_name)


def create_yin_file(yang_file, yin_file):
    yang_file = search_path_for_file(yang_file)
    #print "converting "+yang_file+" to "+ yin_file
    run_cmd([ 'pyang','-o',yin_file,'-f', 'yin',yang_file])

def node_get_desc(module,node):
    d = node.find(module.ns()+'description')
    if d == None:
        return ""
    s = ""
    if d.get('name')!=None:
        s = d.get('name')
    t = d.find(module.ns()+'text')
    if t != None:
        if t.text != None:
            return t.text
    return s


def get_node_tag(module,node):
    return node.tag[len(module.ns()):]

#get the ID for a specific node.This normally is getting the 'name' attribute of the node
def node_get_identifier(node):
    return node.get('name')

#get the node type - if not found return "Und"
def node_get_type(module,node):
    t = node.find(module.ns()+"type")
    if t == None:
        return "Und"
    s = t.get('name')
    if s==None:
        s = "Und"
    return s

def header_file_open(src_file, mod_name, stream):
    stream.write( "\n" )
    stream.write( "/*\n" )
    stream.write( "* source file : "+ src_file +"\n")
    stream.write( "* (c) Copyright 2015 Dell Inc. All Rights Reserved."+"\n" )
    stream.write( "*/" +"\n")
    stream.write( "" +"\n")
    stream.write( "/* OPENSOURCELICENSE */" +"\n")

    stream.write( "#ifndef "+string_to_c_formatted_name(mod_name+"_H")+"\n" )
    stream.write( "#define "+string_to_c_formatted_name(mod_name+"_H") +"\n")
    stream.write( "" +"\n")
    stream.write( "" +"\n")

def header_file_close(stream):
    stream.write("#endif"+"\n" )


#Create a string that can be used is C programs
def string_to_c_formatted_name(s):
    s = s.replace('-','_')
    s = s.replace(':','_')
    return s.upper();

#walk through all of the children of nodes and find nodes of the type mentioned
def find_all_classes_of_types(nodes, list_of_types):
    lst = list()
    for i in nodes.iter():
        if i in list_of_types:
            lst.append(i)
    return lst

#find the children for the current node that are of the types mentioned
def find_child_classes_of_types(nodes, list_of_types):
    lst = list()
    for i in list(nodes):
        if i.tag in list_of_types:
            lst.append(i)
    return lst

#find the parent of the node
def find_parent(node, iter):
    par = None
    for p in node.iter() :
        if iter in p:
            par = p;
            break
    return par

#get the node path
def get_node_path(module,node, root_node):
    s = node_get_identifier(node);
    p = node;
    while True:
        p = find_parent(p,root_node.iter())
        if p == None:
            return s;
        s = node_get_identifier(p)+"/"+s

#generate an index for a node.
class IndexTracker:
    ix = 0
    begin = 0;

    def __init__(self, init=0):
        self.ix = init
        self.begin = self.ix

    def get(self):
        return self.ix

    def inc(self):
        self.ix+=1

    def unused(self) :
        return self.ix == self.begin

