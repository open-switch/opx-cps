import os
import sys

class COutputFormat:

    def __init__(self,context):
        self.context = context
        self.lang = context['output']['language']['cps']

    def show(self,model):
        self.model = model
        self.create_map_src()

    def create_map_src(self):
        print "/*"
        print self.context['model-names'][self.model.module.name()]
        print "*/"
        print "#include \""+self.context['model-names'][self.model.module.name()]+".h\""
        print "#include \"cps_class_map.h\""
        print ""
        print "#include <vector>"
        print ""
        print "static struct {"
        print "  std::vector<cps_api_attr_id_t> _ids;"
        print "  cps_api_attr_id_t id;"
        print "  cps_class_map_node_details details;"
        print "} lst[] = {"
        for i in self.model.keys:

            if i == self.lang.category: continue
            line = "{ {"
            key_str = ""
            for key in self.model.keys[i].split():
                key_str+= self.lang.names[key]+","

            if len(key_str)==0: key_str = self.lang.names[i]+","
            key_str = key_str [:-1]
            line +=key_str+"},"
            line += self.lang.names[i]+","
            line += " { \""
            line += i+"\",\"\","

            if not i in self.model.container_map:
                line+="false"
            else :
                line+="true"
            line+=","
            if i in self.model.all_node_map:
                ele = self.model.all_node_map[i]
                tag = self.model.module.filter_ns(ele.tag)
                if tag == 'leaf-list':
                    line+="CPS_CLASS_ATTR_T_LEAF_LIST|"
                if tag == 'leaf':
                    line+="CPS_CLASS_ATTR_T_LEAF|"
            if i in self.model.container_map:
                line+="CPS_CLASS_ATTR_T_CONTAINER|"
            line+= self.lang.determine_type(i)
            line+="}},"
            print line

        print "};"
        print ""

        print ""
        print "static const size_t lst_len = sizeof(lst)/sizeof(*lst);"
        print "extern \"C\"{ "
        print "  t_std_error module_init(void) {"
        print "    size_t ix = 0;"
        print "    for ( ; ix < lst_len ; ++ix ) { "
        print "        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details); "
        print "    }"
        print "    return STD_ERR_OK;"
        print "  }"
        print "}"

