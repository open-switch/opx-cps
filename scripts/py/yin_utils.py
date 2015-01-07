# This file contains a few general purpose YIN utilities

import yin_ns

def node_get_desc(node):
    d = node.find(yin_ns.get_ns()+'description')
    if d == None:
        return ""
    s = ""
    if d.get('name')!=None:
        s = d.get('name')
    t = d.find(yin_ns.get_ns()+'text')
    if t != None:
        if t.text != None:
            return t.text
    return s    
    

#get the ID for a specific node.This normally is getting the 'name' attribute of the node
def node_get_identifier(node):
    id = node.get('name')
    if id == None:
        return node.tag[len(yin_ns.get_ns()):]
    return id

#get the node type - if not found return "Und"
def node_get_type(node):
    t = node.find(yin_ns.get_ns()+"type")
    if t == None:
        return "Und"
    s = t.get('name')
    if s==None:
        s = "Und"
    return s

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
def get_node_path(node, root_node):
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

