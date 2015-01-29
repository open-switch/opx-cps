
mod_ns = ""
module_name = ""

def get_namespace(node):
    tag = node.tag;
    lst = tag.rsplit("}");
    return lst[0] + "}"

def get_ns():
    return mod_ns

def get_mod_name():
    return module_name

def set_mod_name(node):
    global module_name
    if node.tag == get_ns()+'module':
        module_name = node.get('name')

def yin_ns_init(node):
    global mod_ns
    mod_ns = get_namespace(node)

#Create a list that also has the NS prefix to the names
def prepend_ns_to_list(types):
    l = list()
    for elem in types:
        l.append(get_ns() + elem)
    return l
