
import yin_utils
import object_history

import sys


class Element:
    name = ""
    node = None
    description = ""
    
    
    def __init__(self, node,module,history):
        self.node = node
        self.name = node.get('name')
        self.module = module
        self.history = history

    def create(node):
        return Element(node)

    create = staticmethod(create)

    def get_description(self):
        n = self.node.find(self.module.ns()+'description')
        if n != None:
            n = n.find(self.module.ns()+'text')
            if n != None:
                self.description = n.text


class Enumeration(Element):
    enums = None
    module = None
    def load_enums(self):
        self.enums = dict()
        n = self.node.find(self.module.ns()+'type')
        if n == None: return
        enums = list(n)

        typename = yin_utils.string_to_c_formatted_name(self.name)+"_t"

        has_value = False
        for e in enums:
            enum_name = self.name+"-"+e.get('name')

            value = e.find('value')
            en = self.history.get_enum(typename)

            if value != None:
                enum_val = int(value.get('value'))
                en.add_enum(enum_name,enum_val)
                has_value = True

            else :
                enum_val = en.get_value(enum_name)

            self.enums[enum_val] = enum_name


    def __init__(self, node, module, history):
        Element.__init__(self, node,module,history)    
        self.get_description()
        self.load_enums()

    def to_string(self):
        s = "typedef enum { \n"
        for e in self.enums.keys():
            s+= "  " + yin_utils.string_to_c_formatted_name(self.enums.get(e)) +" = "+ str(e)+",\n"
        s+= "} "+ yin_utils.string_to_c_formatted_name(self.name)+"_t" +" ; \n"
        return s

    def create(node):
        return Enumeration(node)

    create = staticmethod(create)



class EnumerationList:
    enum = None
    module = None
    history = None
    
    def __init__(self, node, module, history):
        self.enum = list()
        self.module = module
        self.history = history        
        self.find_all(node)

    def find_all(self,node):
        for i in node.iter():
            if i.tag == self.module.ns()+"type" and i.get('name').lower()== 'enumeration':
                par = yin_utils.find_parent(node,i)
                self.enum.append(Enumeration(par,self.module,self.history))

    def print_source(self):
        for i in self.enum:
            print i.to_string()

