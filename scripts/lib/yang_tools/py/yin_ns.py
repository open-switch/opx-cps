
import object_history 
import os


def get_namespace(node):
    tag = node.tag;
    lst = tag.rsplit("}");
    return lst[0] + "}"

def set_mod_name(ns,node):
    n = node.find(ns+'prefix')
    if n == None:
        if node.tag == ns+'module':
            return node.get('name')
    else:
        return n.get('value')
    return ""


class Module:
    mod_ns = ""
    module_name = ""
    filename = ""

    def __init__(self,filename, node):
        self.filename = filename
        self.mod_ns =get_namespace(node)
        self.module_name = set_mod_name(self.mod_ns,node);
       
    def get_file(self):
        return os.path.basename(self.filename)
    
    def ns(self):
        return self.mod_ns

    def name(self):
        return self.module_name
    
    #Create a list that also has the NS prefix to the names
    def prepend_ns_to_list(self,types):
        l = list()
        for elem in types:
            l.append(self.ns() + elem)
        return l

