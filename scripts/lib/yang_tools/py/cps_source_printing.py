
import os
import sys
import yin_utils

def string_to_c_formatted_name(s):
    s = s.replace('-','_')
    s = s.replace(':','_')
    return s.upper();

class COutputFormat:

    def node_get_text(self,model,node):
        node = node.find(model.module.ns()+'text')
        if node!=None:
            node = node.text
        if node == None:
            node = ""
        return node

    def get_type(self,model,node):
        type = node.find(model.module.ns()+'type')
        if type==None: return ""
        return type.get('name')

    def get_comment(self,model, node,enclose=True):
        comments = node.find(model.module.ns()+'description')
        if comments!=None:
            comments = self.node_get_text(model,comments)
        else:
            comments=""
        comments = comments.strip()
        if len(comments) > 0:
            if enclose==True: comments = "/*"+comments+"*/"
        return comments

    def get_value(self,model,node):
        value = node.find(model.module.ns()+'value')
        if value!=None:
            value = value.get('value')
        if value!=None and len(value)==0: value=None
        return value

    def __init__(self,context):
        self.context = context

    def show_enum(self,model,name):

        node = model.context['enum'][name]
        enum = node.find('enumeration')
        print ""
        print "/*Enumeration "+name+" */"
        print "typedef { "
        for i in node.iter():
            if model.module.filter_ns(i.tag) == 'enum':
                en_name = string_to_c_formatted_name(name+"_"+i.get('name'))
                value = self.get_value(model,i)
                value = str(model.history.get_enum(name,en_name,value))
                comment = self.get_comment(model,i)
                print "  "+en_name+"="+value+", "+comment

        print "} "+string_to_c_formatted_name(name)+"_t;"

    def print_enums(self,model):
        name = model.module.name()
        for i in model.context['enum'].keys():
            if i.find(name)==-1: continue
            self.show_enum(model,i)

    def print_container(self,model):
        for name in model.container_map.keys():
            if name == model.module.name(): continue
            node = model.container_map[name]
            if len(node)==0: continue

            print ""
            print "/*Object "+name+" */"
            print "typedef { "
            for c in node:
                if c.name == model.module.name(): continue
                en_name = string_to_c_formatted_name(c.name)
                value = str(model.history.get_enum(name,en_name,None))
                comment = self.get_comment(model,c.node)
                print  "/*type="+self.get_type(model,c.node)+"*/ "
                if len(comment)>0: print comment
                print "  "+en_name+"="+value+","
                print ""
            print "} "+string_to_c_formatted_name(name)+"_t;"

        print "/* Object subcategories */"
        subcat_name = model.module.name()+"_objects"
        print "typedef {"
        for c in model.container_map[model.module.name()]:
            name = c.name
            node = model.container_map[name]
            en_name = string_to_c_formatted_name(name+"_obj")
            value = str(model.history.get_enum(model.module.name(),en_name,None))
            comment = self.get_comment(model,c.node)
            if len(comment)>0: print comment
            print "  "+en_name+"="+value+","
            print ""

        print "} "+string_to_c_formatted_name(subcat_name)+"_t;"
        print ""

    def print_types(self,model):
        name = model.module.name()
        for i in model.context['types'].keys():
            if i.find(name)==-1: continue
            if i in model.context['enum']: continue

            i = model.context['types'][i]
            if i.tag == model.module.ns()+'grouping': continue
            print "/* "
            type = i.find(model.module.ns()+'type')
            print "Name: "+ i.get('name')
            if type!=None:
                print "Type:"+ type.get('name')
            print "Comments:"+self.get_comment(model,i,enclose=False)
            print "*/\n"


    def show(self,model):
        yin_utils.header_file_open(model.module.name(),model.module.name(),sys.stdout)

        print ""
        id = model.history.get_category(model.module.name())

        print "#define cps_api_obj_CAT_"+string_to_c_formatted_name(model.module.name())+" ("+str(id)+") "

        self.print_types(model)
        self.print_enums(model)
        self.print_container(model)

        yin_utils.header_file_close(sys.stdout)

