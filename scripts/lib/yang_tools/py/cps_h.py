
import os
import sys
import yin_utils

def to_c_type(context,elem):
    return "cps_api_object_ATTR_T_BIN"

class COutputFormat:

    def node_get_text(self,model,node):
        node = node.find(model.module.ns()+'text')
        if node!=None:
            node = node.text
        if node == None:
            node = ""
        return node

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
        self.lang = self.context['output']['language']['cps']

    def show_enum(self,model,name):
        history = self.lang.history

        node = model.context['enum'][name]
        enum = node.find('enumeration')
        print ""
        print "/*Enumeration "+name+" */"
        print "typedef enum { "
        for i in node.iter():
            if model.module.filter_ns(i.tag) == 'enum':
                en_name = self.lang.to_string(name+"_"+i.get('name'))
                value = self.get_value(model,i)
                value = str(history.get_enum(en_name,value))
                comment = self.get_comment(model,i)
                print "  "+en_name+" = "+value+", "+comment

        print "} "+self.lang.to_string(name)+"_t;"

    def print_enums(self,model):
        name = model.module.name()
        for i in model.context['enum'].keys():
            if i.find(name)==-1: continue
            self.show_enum(model,i)

    def print_container(self,model):
        history = self.lang.history

        for name in model.container_map.keys():
            if name == model.module.name(): continue

            node = model.container_map[name]
            if len(node)==0: continue

            print ""
            print "/*Object "+name+" */"
            print "typedef enum { "
            for c in node:
                if c.name == model.module.name(): continue
                en_name = self.lang.to_string(c.name)
                value = str(history.get_enum(en_name,None))
                comment = self.get_comment(model,c.node)
                print  "/*type="+self.lang.get_type(c.node)+"*/ "
                if len(comment)>0: print comment
                print "  "+en_name+" = "+value+","
                print ""
            print "} "+self.lang.to_string(name)+"_t;"

        print ""

        if len(model.container_map[model.module.name()])==0:
            print "/*No objects defined...*/"
            return

        print "/* Object subcategories */"
        subcat_name = model.module.name()+"_objects"
        print "typedef enum{"
        for c in model.container_map[model.module.name()]:
            comment = ""
            en_name = self.lang.names[c.name]
            value = str(history.get_enum(en_name,None))
            if c.name in model.container_map:
                node = model.container_map[c.name]
                comment = self.get_comment(model,c.node)
            if len(comment)>0: print comment
            print "  "+en_name+" = "+value+","
            print ""

        print "} "+self.lang.to_string(subcat_name)+"_t;"
        print ""
        print ""

    def dump_keys(self,model):

        print ""
        print "/*"
        print "All nodes that contains keys follow:"
        for i in model.container_keys.keys():
            print i
        print "*/"
        print ""
        for i in self.lang.keys.keys():
            key_args = self.lang.keys[i].split(',')
            model_keys = key_args[:2]
            remaining_keys = key_args[2:]

            line = "static inline void "+self.lang.to_string(i)+"_key(cps_api_key_t *key, "
            line+= "cps_api_qualifier_t qual) {"
            print line
            line = "  cps_api_key_init(key,qual,"+",".join(model_keys)+","
            line+=str(len(remaining_keys))+","
            line += ",".join(remaining_keys)
            line+=");"
            print line
            print "}"
            print ""


    def print_types(self,model):
        print ""

        for i in model.context['types'].keys():
            name = model.module.name()
            if i.find(name)==-1: continue
            if i in model.context['enum']: continue #already printed

            node = model.context['types'][i]
            if node.tag == model.module.ns()+'grouping': continue #not printable

            type = self.lang.get_type(node)

            if self.lang.valid_lang_type(type):
                comments = self.get_comment(model,node,enclose=False)
                if len(comments) > 0:
                    print "/*"
                    print "Comments:"+comments
                    print "*/"
                print 'typedef '+self.lang.type_to_lang_type(type)+' '+self.lang.to_string(i)+"_t;"
            else:
                print "/* "
                print "Name: "+ i
                if type!=None:
                    print "Type:"+ type
                print "Comments:"+self.get_comment(model,node,enclose=False)
                print "*/\n"
        print ""

    def header_file_open(self,model, stream):
        src_file = model.module.name()
        mod_name = model.module.name()

        stream.write( "\n" )
        stream.write( "/*\n" )
        stream.write( "* source file : "+ src_file +"\n")
        stream.write( "* (c) Copyright 2015 Dell Inc. All Rights Reserved."+"\n" )
        stream.write( "*/" +"\n")
        stream.write( "" +"\n")
        stream.write( "/* OPENSOURCELICENSE */" +"\n")

        stream.write( "#ifndef "+self.lang.to_string(mod_name+"_H")+"\n" )
        stream.write( "#define "+self.lang.to_string(mod_name+"_H") +"\n")
        stream.write( "" +"\n")
        stream.write( "#include \"cps_api_operation.h\"\n")

        module = model.imports['module']

        for i in module:
            if i.find('dell')==-1: continue
            stream.write( "#include \""+i+".h\"\n")

        stream.write( "#include <stdint.h>\n")
        stream.write( "#include <stdbool.h>\n")
        stream.write( "" +"\n")


    def header_file_close(self,stream):
        stream.write("#endif"+"\n" )

    def show(self,model):
        self.header_file_open(model,sys.stdout)

        print ""
        id = self.lang.history.get_global(self.lang.get_category())

        print "#define "+self.lang.get_category()+" ("+str(id)+") "

        self.print_types(model)
        self.print_enums(model)
        self.print_container(model)

        self.dump_keys(model)
        self.header_file_close(sys.stdout)

