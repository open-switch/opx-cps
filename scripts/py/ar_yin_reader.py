
import yin_ns
import yin_utils
import yin_enum

import xml.etree.ElementTree as ET

import sys

import yin_cps


mod_prefix = ""



factory = { 
           'enumeration' : yin_enum.Enumeration.create,
           'element' : yin_enum.Element.create,
           }

def handle_typedef(node):
    t = node.find(yin_ns.get_ns()+'type')
    if t == None:
        return None
    
    typedef_type = t.get('name')
    
    if typedef_type == None:
        return None
    
    return factory[typedef_type](node)
    

node_handlers = {
                 'typedef' : handle_typedef                 
                 }



if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Missing arguements need yin formatted file"

    et = ET.parse(sys.argv[1])    
    root = et.getroot()
    
    just_name = sys.argv[1]
    if just_name.rfind('/')!=-1:
        just_name = just_name[just_name.rfind('/')+1:]
    
    #Iniitalize the NS 
    yin_ns.yin_ns_init(root)
    yin_ns.set_mod_name(root)


    nodes = list(root)
    print ""
    print "/*"
    print "* source yang file : "+ just_name
    print "* (c) Copyright 2014 Dell Inc. All Rights Reserved."
    print "*/"
    print ""
    print "/* OPENSOURCELICENSE */"

    print "#ifndef "+yin_utils.string_to_c_formatted_name(yin_ns.get_mod_name()+"_H")
    print "#define "+yin_utils.string_to_c_formatted_name(yin_ns.get_mod_name()+"_H")
    print ""
    print ""
    
    yin_cps.cps_create_doc_ids(root)    
    el = yin_enum.EnumerationList(root) 
    el.show()
    
    print "#endif"
      
    sys.exit(0)
    