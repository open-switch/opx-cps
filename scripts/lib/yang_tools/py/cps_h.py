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

import os
import sys
import yin_utils


def to_c_type(context, elem):
    return "cps_api_object_ATTR_T_BIN"


class COutputFormat:

    """Responsible for generating header files with enums, etc.."""

    def print_comment(self, comment):
        print("/* %s */\n" % (comment))

    def node_get_text(self, model, node):
        node = node.find(model.module.ns() + 'text')
        if node is not None:
            node = node.text
        if node is None:
            node = ""
        return node

    def get_comment(self, model, node, enclose=True):
        comments = node.find(model.module.ns() + 'description')
        if comments is not None:
            comments = self.node_get_text(model, comments)
        else:
            comments = ""
        comments = comments.strip()
        if len(comments) > 0:
            if enclose:
                comments = "/*" + comments + "*/"
        return comments

    def get_value(self, model, node):
        value = node.find(model.module.ns() + 'value')
        if value is not None:
            value = value.get('value')
        if value is not None and len(value) == 0:
            value = None
        return value

    def __init__(self, context):
        self.context = context
        self.lang = self.context['output']['language']['cps']

    def show_enum(self, model, name):
        history = self.lang.history

        node = model.context['enum'][name]
        if model.module.filter_ns(node.tag) != 'typedef':
            node = node.find(model.module.ns() + 'type')

        enum = node.find('enumeration')

        print ""
        self.print_comment('Enumeration ' + name)
        for i in [node, enum]:
            if i is None:
                continue
            _txt = self.get_comment(model, i)
            if _txt is not None and len(_txt) > 0:
                print _txt

        print "typedef enum { "

        min_value = None
        max_value = None

        for i in node.iter():
            if model.module.filter_ns(i.tag) == 'enum':
                en_name = self.lang.to_string(name + "_" + i.get('name'))
                value = self.get_value(model, i)
                if value is not None:
                    value = long(value)
                value = history.add_enum(name, en_name, value)

                if min_value is None or min_value > value:
                    min_value = value
                if max_value is None or max_value < value:
                    max_value = value

                comment = self.get_comment(model, i)
                print "  " + en_name + " = " + str(value) + ", " + comment

        print ("  %s_%s=%s," % (self.lang.to_string(name), 'MIN', min_value))
        print ("  %s_%s=%s," % (self.lang.to_string(name), 'MAX', max_value))
        print "} " + self.lang.to_string(name) + "_t;"

    def print_enums(self, model):
        name = model.module.name()
        for i in model.context['enum'].keys():
            if i.find(name + ':') != 0:
                continue
            node = model.context['enum'][i]
            type_name = node.find(model.module.ns() + 'type')
            if type_name is not None:
                type_name = type_name.get('name')

            if type_name is not None and type_name != 'enumeration':
                print "Skipping %s due to non enum" % type_name
                continue

            self.show_enum(model, i)

    def get_attr_name(self, model, node):
        l = []
        for i in node.findall(model.module.ns() + 'type'):
            l.append(i.get('name'))
        return l

    def resolve_node_names(self, model, node, name):

        _node_type_inst = node.find(model.module.ns() + 'type')
        _node_type = self.context.resolve_type(_node_type_inst)

        if _node_type is None:
            return [(name, 'binary')]

        if _node_type == 'union':
            _node_type_inst = self.context.resolve_type(_node_type_inst, False)
            _found_names = []
            for _types in _node_type_inst.findall(model.module.ns() + 'type'):
                _new_name = name
                if _types.get('name') is not None:
                    _new_name = _new_name + '/' + _types.get('name')
                    if self.context.name_is_a_type(_types.get('name')):
                        _found_names = _found_names + \
                            self.resolve_node_names(
                                model,
                                self.context.get_tyoe_node(_types.get('name')),
                                _new_name)
                    else:
                        _found_names = _found_names + [(_new_name, _new_name)]
            return _found_names

        if _node_type is not None:
            return [(name, _node_type)]

        return []

    def print_container(self, model):
        history = self.lang.history
        __category = self.lang.get_category()

        for name in model.container_map.keys():
            if name == model.module.name():
                continue

            node = model.container_map[name]
            if len(node) == 0:
                continue

            self.print_comment('Object ' + name)
            print "typedef enum { "
            for c in node:

                if c.name == model.module.name():
                    continue

                _names = self.resolve_node_names(model, c.node, c.name)

                comment = self.get_comment(model, c.node)
                print (comment)
                _set = set()
                _count = 0
                for _name in _names:
                    _count += 1
                    en_name = self.lang.to_string(_name[0])
                    if en_name in _set:
                        en_name = en_name + '_' + str(_count)
                    _set.add(en_name)
                    value = str(history.add_enum(__category, en_name, None))
                    print "/*type=" + _name[1] + "*/ "
                    print "  " + en_name + " = " + value + ","

                if self.lang.to_string(c.name) not in _set:
                    en_name = self.lang.to_string(c.name)
                    value = str(history.add_enum(__category, en_name, None))
                    print "  " + en_name + " = " + value + ","

            print "} " + self.lang.to_string(name) + "_t;"

        print ""

        if len(model.container_map[model.module.name()]) == 0:
            self.print_comment('No objects defined..')
            return

        self.print_comment('Top level objects')

        subcat_name = model.module.name() + "_objects"
        print "typedef enum { "
        for c in model.container_map[model.module.name()]:
            comment = ""

            __en_value = ''
            __en_name = ''
            __en_alias = ''

            if c.name + '_##alias' in self.lang.names:
                __en_alias = self.lang.names[c.name + '_##alias']

            __en_name = self.lang.names[c.name]

            if not __en_alias == '':
                __en_value = str(
                    history.add_enum(
                        __category,
                        __en_alias,
                        None))
            else:
                __en_value = str(history.add_enum(__category, __en_name, None))

            if c.name in model.container_map:
                comment = self.get_comment(model, c.node)
            if len(comment) > 0:
                print comment
            print "  " + __en_name + " = " + __en_value + ","
            if __en_alias != '':
                print "  " + __en_alias + " = " + __en_value + ","

            print ""

        print "} " + self.lang.to_string(subcat_name) + "_t;"
        print ""
        print ""

    def print_types(self, model):
        print ""
        name = model.module.name()
        history = self.lang.history
        for i in model.context['types'].keys():
            # find local model only
            if i.find(name + ':') != 0:
                continue

            if i in model.context['enum']:
                continue  # already printed

            node = model.context['types'][i]
            if node.tag == model.module.ns() + 'grouping':
                continue  # not printable

            if node.tag == model.module.ns() + 'identity':
                en_name = self.lang.to_string(node.get('__identity__'))
                comment = self.get_comment(model, node)

                if len(comment) > 0:
                    print(comment)

                print(
                    '#define ' +
                    en_name +
                    '  \"' +
                    model.module.name(
                    ) +
                    ':' +
                    node.get(
                        'name') +
                    '\"')
                continue

            _type = self.lang.get_type(node)

            if self.lang.valid_lang_type(_type):
                comments = self.get_comment(model, node, enclose=False)
                if len(comments) > 0:
                    print "/*"
                    print "Comments:" + comments
                    print "*/"
                print 'typedef ' + self.lang.type_to_lang_type(_type) + ' ' + self.lang.to_string(i) + "_t;"
            else:
                print "/* "
                print "Name: " + i
                if _type is not None:
                    print "Type:" + _type
                print "Comments:" + self.get_comment(model, node, enclose=False)
                print "*/\n"
        print ""

    def header_file_includes(self, model, stream):
        stream.write("" + "\n")
        module = model.imports['module']

        for i in module:
            if i.find('dell') == -1 \
                and i.find('opx') == -1 \
                and i.find('nvo') == -1:
                #for now as not all models are being built
                #have to customize the list of included headers - reducing to the following set
                #will have this either passed in by command line or removed in the future
                if i.find('ietf-ip') == -1 \
                    and i.find('ietf-interfaces') == -1 \
                    and i.find('ietf-routing') == -1 \
                    and i.find('ietf-network-instance') == -1:
                    continue
            stream.write("#include \"" + i + ".h\"\n")

        stream.write("" + "\n")

    def show(self, model):
        import c_utils
        c_utils.header_file_open(
            model.module.model_name(),
            model.module.model_name(),
            sys.stdout)
        self.header_file_includes(model, sys.stdout)

        print ""
        id = self.context['history'][
            'category'].get_category(self.lang.get_category())

        print "#define " + self.lang.get_category() + " (" + str(id) + ") "

        model_name = model.module.model_name()
        print "\n#define " + self.lang.to_string(model_name) + "_MODEL_STR" + " \"" + model_name + "\""
        self.print_types(model)
        self.print_enums(model)
        self.print_container(model)

        c_utils.header_file_close(sys.stdout)
