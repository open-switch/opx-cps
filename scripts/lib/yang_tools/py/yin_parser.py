
import yin_ns
import yin_utils
import yin_enum
import object_history

import yin_model

import xml.etree.ElementTree as ET

import os
import sys


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

def process(filename):    
    cps = yin_cps.CPSParser(filename)
    cps.parse()
    cps.close()
    yin_utils.header_file_open(cps.module.get_file(),cps.module.name(),sys.stdout)
    
    cps.print_source()
            
    yin_utils.header_file_close(sys.stdout)

    

if __name__ == '__main__':    
    if len(sys.argv) < 2:
        print "Missing arguements need yin formatted file"
    print "YANG_PATH = "+os.environ.get('YANG_PATH',"")
    
    yf = yin_model.CPSYangModel(sys.argv[1]);
    
    sys.exit(0)
