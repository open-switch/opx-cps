import yin_ns
import yin_utils
import object_history

import sys



class Element:
    name = ""
    node = None
    description = ""

    def __init__(self, node):
        self.node = node
        self.name = node.get('name')

    def create(node):
        return Element(node)

    create = staticmethod(create)

    def get_description(self):
        n = self.node.find(yin_ns.get_ns()+'description')
        if n != None:
            n = n.find(yin_ns.get_ns()+'text')
            if n != None:
                self.description = n.text


class Enumeration(Element):
    enums = None

    def load_enums(self):
        self.enums = dict()
        n = self.node.find(yin_ns.get_ns()+'type')
        if n == None: return
        enums = list(n)

        typename = yin_utils.string_to_c_formatted_name(self.name)+"_t"

        has_value = False
        for e in enums:
            enum_name = self.name+"-"+e.get('name')

            value = e.find('value')
            en = object_history.get().get_enum(typename)

            if value != None:
                enum_val = int(value.get('value'))
                en.add_enum(enum_name,enum_val)
                has_value = True

            else :
                enum_val = en.get_value(enum_name)

            self.enums[enum_val] = enum_name


    def __init__(self, node):
        Element.__init__(self, node)
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
    def __init__(self,node):
        self.enum = list()
        self.find_all(node)

    def find_all(self,node):
        for i in node.iter():
            if i.tag == yin_ns.get_ns()+"type" and i.get('name').lower()== 'enumeration':
                par = yin_utils.find_parent(node,i)
                self.enum.append(Enumeration(par))

    def show(self):
        for i in self.enum:
            print i.to_string()

