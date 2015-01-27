
import yin_ns
import yin_utils
import yin_enum
import object_history
import yin_cps

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

def process(filename,history):
    et = ET.parse(filename)
    root = et.getroot()

    object_history.init(history)

    just_name = os.path.basename(filename)

    #Iniitalize the NS
    yin_ns.yin_ns_init(root)
    yin_ns.set_mod_name(root)


    nodes = list(root)
    yin_utils.header_file_open(just_name,yin_ns.get_mod_name(),sys.stdout)

    yin_cps.cps_create_doc_ids(root)
    el = yin_enum.EnumerationList(root)
    el.show()
    yin_utils.header_file_close(sys.stdout)

    object_history.close()


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Missing arguements need yin formatted file"
    process(sys.argv[1],sys.argv[1]+".hist")
    sys.exit(0)
