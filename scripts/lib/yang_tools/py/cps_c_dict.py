#
# Copyright (c) 2018 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

import sys

import yin_utils
import c_utils


class COutputFormat:

    def __init__(self, context):
        self.context = context
        self.lang = context['output']['language']['cps']

    def show(self, model):
        self.model = model
        self.create_map_src()

    def create_map_src(self):
        c_utils.add_copyright_to_file(sys.stdout)

        _max_len = 0
        #generate static length as a max for all keys...
        for i in self.model.keys:
            if i == self.lang.category:
                continue

            # start the key encoding...
            key_str = ""
            _lst = self.model.keys[i].split()

            if _max_len < len(_lst)+1:
                _max_len = len(_lst)+1


        print "/*"
        print self.context['model-names'][self.model.module.name()]
        print "*/"
        print "#include \"" + self.context['model-names'][self.model.module.name()] + ".h\""
        print "#include \"cps_class_map.h\""
        print ""

        _len_elems = 0
        print "static const struct {"
        print "  cps_api_attr_id_t ids["+str(_max_len)+"]; //maximum of any keys in this file"
        print "  size_t ids_size;"
        print "  cps_api_attr_id_t id;"
        print "} _keys[] = {"

        for i in self.model.keys:

            if i == self.lang.category:
                continue

            ele = None
            tag = None

            if i not in self.model.all_node_map:
                continue

            ele = self.model.all_node_map[i]
            tag = self.model.module.filter_ns(ele.tag)

            # start the key encoding...
            key_str = ""
            for key in self.model.keys[i].split():
                key_str += self.lang.names[key] + ","

            if len(key_str) == 0:
                key_str = self.lang.names[i] + ","
            key_str = key_str[:-1]

            line = "{ { %s }, %d, %s}, " % (key_str, len(self.model.keys[i].split()), self.lang.names[i])
            print line
            _len_elems+=1;
        print "};"
        print ""

        print "static const cps_class_map_node_details _details[/*"+str(_len_elems)+"*/] = {"
        for i in self.model.keys:

            if i == self.lang.category:
                continue

            ele = None
            tag = None

            if i not in self.model.all_node_map:
                continue

            ele = self.model.all_node_map[i]
            tag = self.model.module.filter_ns(ele.tag)

            # start the key encoding...
            key_str = ""
            for key in self.model.keys[i].split():
                key_str += self.lang.names[key] + ","

            if len(key_str) == 0:
                key_str = self.lang.names[i] + ","
            key_str = key_str[:-1]

            # Create the structure contianing type and etc details
            _n_name = i
            _n_desc = yin_utils.node_get_desc(self.model.module, ele)
            _n_desc = _n_desc.translate(None, '\n#![]$\"')
            _n_emb = 'false'

            if i in self.model.container_map:
                _n_emb = "true"

            _n_attr_type = 'CPS_CLASS_ATTR_T_CONTAINER'
            if tag == 'leaf-list':
                _n_attr_type = "CPS_CLASS_ATTR_T_LEAF_LIST"
            elif tag == 'leaf':
                _n_attr_type = "CPS_CLASS_ATTR_T_LEAF"
            elif tag == 'list':
                _n_attr_type = "CPS_CLASS_ATTR_T_LIST"

            _n_data_type = self.lang.cps_map_type(self.context, ele)

            line = "{ \"%s\", \"%s\", %s, %s, %s }," % (
                _n_name,
                _n_desc,
                _n_emb,
                _n_attr_type,
                _n_data_type)

            print line

        print "};"
        print ""

        print ""
        print "static const size_t lst_len = sizeof(_keys)/sizeof(*_keys);"
        print "extern \"C\"{ "
        print "  t_std_error module_init(void) {"
        print "    size_t ix = 0;"
        print "    for ( ; ix < lst_len ; ++ix ) { "
        print "        cps_class_map_init(_keys[ix].id,_keys[ix].ids,_keys[ix].ids_size,_details+ix); "
        print "    }"
        print "    return STD_ERR_OK;"
        print "  }"
        print "}"
