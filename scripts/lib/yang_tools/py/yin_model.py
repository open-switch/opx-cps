#
# Copyright (c) 2019 Dell Inc.
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
import os

import yin_cps
import cps_c_lang
import cms_lang
import cps_context
import object_history


class CPSYinFiles:
    yin_map = dict()
    yin_parsed_map = dict()

    def __init__(self, context):
        self.tmpdir = context['temp-dir']
        self.context = context

    def check_deps_loaded(self, module, context):
        if module not in self.context['module']:
            return False

        __mod = self.context['module']

        for i in __mod.imports:
            i = os.path.splitext(i)[0]
            if not i in context['current_depends']:
                return False
        return True

    def load_depends(self, module, context):
        entry = self.yin_map[module]
        for i in entry.imports:
            i = os.path.splitext(i)[0]
            if i in context['current_depends']:
                continue
            if not self.check_deps_loaded(i, context):
                self.load_depends(i, context)
            if i in context['current_depends']:
                raise Exception("")
            context['current_depends'].append(i)

        if module in context['current_depends']:
            return

        context['current_depends'].append(module)

    def get_parsed_yin(self, filename, prefix):
        _key= filename
        _key = _key.replace('.yang','')

        if prefix is not None:
            _key += ":" + prefix

        self.context.add_model_name(_key,filename)

        _model = self.context.get_model(_key)

        if _model is None:
            _model = yin_cps.CPSParser(self.context, _key,filename)

            _model.load(prefix=prefix)
            _model.walk()

            self.context.add_model(_key,_model)

            self.context['model-names'][self.context.get_model(_key).module.name()] = _key
            self.context['model-names'][self.context.get_model(_key).module.name()+'_model_'] = self.context.get_model(_key)

            #TODO remove
            self.yin_map[_key] = self.context.get_model(_key)

        return _model


    def load(self, yang_file, prefix=None):
        """Convert the yang file to a yin file and load the model"""
        return self.get_parsed_yin(yang_file, prefix)


import cps_context

class CPSYangModel:

    def __init__(self, args):
        self.model = None
        self.context = cps_context.context(args)

        self.filename = self.context.get_arg('file')
        if len(self.filename)==0:
            print('Missing parameters.  Please specify file=[target file], eg file=zzz.yang')
            sys.exit(1)

        self.coutput = None

        #These are the output plugins for this parser
        self.context['output']['language'] = {
            'cps' : cps_c_lang.Language(self.context),
            'cms' : cms_lang.Language(self.context) }

        __cat_file_name = self.context.get_config_path('category.yconf')

        object_history.YangHistory_CategoryParser(self.context,__cat_file_name)

        #Filled in during initialization the root yang model parser
        #self.context['history']['file']

        #delete next cleanup
        self.context['loader'] = CPSYinFiles(self.context)

        self.model = self.context['loader'].load(self.filename)

        #assume order independent modules
        for i in self.context['output']['language'].values():
            i.setup(self.model.get_key())

        for plugin in self.context.get_arg('output').split(','):
            if plugin in self.context['output']['language'].keys():
                self.context['output']['language'][plugin].write()


    def write_details(self, key):
        class_type = self.context['output'][key]
        if key in self.args:
            old_stdout = sys.stdout
            with open(self.args[key], "w") as sys.stdout:
                class_type.COutputFormat(self.context).show(self.model)
            sys.stdout = old_stdout

    def close(self):
        for i in self.context['output']['language']:
            self.context['output']['language'][i].close()
        self.context['history']['category'].write()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
       self.context.clear()