import os
import sys

class COutputFormat:

    def __init__(self,context):
        self.context = context

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
        print "  cps_class_map_node_details details;"
        print "} lst[] = {"
        for i in self.model.name_to_id.keys():
            line = "{"
            line += self.model.path_to_ids(i)+", { \""
            line += i+"\",\"\","

            if not i in self.model.container_map:
                line+="false"
            else :
                line+="true"
            line+=","
            line+= self.context['ctype'](self.context,i)
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
        print "        cps_class_map_init(&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details); "
        print "    }"
        print "    return STD_ERR_OK;"
        print "  }"
        print "}"

