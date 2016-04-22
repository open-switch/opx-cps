#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN #AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
#  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

import sys
import yin_utils
from os import walk
import os


class enum_tracker_int:

    def __init__(self, indexer, name):
        self.the_dict = dict()
        self.indexer = indexer
        self.the_name = name

    def add_enum(self, name, index):
        if index > int('ffff', 16):
            raise Exception("Invalid id detected " + str(index))

        self.indexer.use_enum(index)
        self.the_dict[name] = index

    def get_name(self):
        return self.the_name

    def get_value(self, name, requested):
        if not name in self.the_dict.keys():
            if requested is not None:
                ix = int(requested)
            else:
                ix = self.indexer.get_free_enum()
            self.add_enum(name, ix)
        if requested is not None:
            self.the_dict[name] = int(requested)
        return self.the_dict[name]

    def setup(self, dic, must_be_unique=False):
        for en in dic:

            val = int(dic[en]) & int("ffff", 16)
            if must_be_unique:
                for e in self.the_dict:
                    if self.the_dict[e] == val:
                        raise Exception(
                            "Found duplicate entries... " + str(
                                val) + " A:" + e + " B:" + en)
            self.add_enum(en, val)


class IndexFile:

    def __init__(self, file):
        self.file = file

    def write(self, enum):
        file = self.file
        file.write("++obj " + enum.the_name + "\n")
        for k in sorted(enum.the_dict.keys()):
            file.write(k + "=" + str(enum.the_dict[k]) + "\n")
        file.write("--obj " + enum.the_name + " end\n")

    def get(self):
        file = self.file
        for l in file:
            if l.find("#") == 0:
                continue
            if l.find("++obj ") == -1:
                continue
            line = l.split()
            if len(line) < 2:
                print("Invalid line in file " + l)
                sys.exit(1)

            the_name = line[1]
            d = {'name': line[1]}
            d['list'] = {}
            for el in file:
                if el.find("--obj ") != -1:
                    break
                if el.find("=") == -1:
                    continue
                line = el.split("=")
                if len(line) < 2:
                    print("Badly formatted line " + line)
                    sys.exit(1)
                d['list'][line[0]] = line[1]

            return d
        return {}


class IndexTracker:

    def __init__(self, start):
        self.last_index = 0
        if start is not None:
            self.last_index = start

    def get_free_enum(self):
        return self.last_index

    def use_enum(self, index):
        if int(self.last_index) < index + 1:
            self.last_index = index + 1
        return index


class history:
    the_file = None
    the_name = ""
    the_dict = None
    object_cat = []
    parsed_files = set()

    GLOBAL_SECTION = "global"
    MODULE_SECTION = "module"

    GLOBAL_enums = enum_tracker_int(IndexTracker(10), "global")

    def get_global_enums(self, filename):
        with open(filename, "r") as f:
            ix_reader = IndexFile(f)
            while True:
                d = ix_reader.get()
                if 'name' not in d:
                    break
                if d['name'] != self.GLOBAL_SECTION:
                    continue

                self.GLOBAL_enums.setup(d['list'], must_be_unique=True)

                cur_enums = self.GLOBAL_enums.the_dict

                break

    def load_all_globals(self):
        l = []
        path = self.context['history']['output']
        f = []
        found_names = []
        for i in path.split(':'):
            for (dirpath, dirnames, filenames) in os.walk(i):
                for filename in filenames:
                    if filename.find('.yhist') == -1:
                        continue

                    if filename in found_names:
                        continue
                        # print("Error.. found two instances of " + filename + " at "+dirpath)
                        # print("other files found are:")
                        # print f
                        # sys.exit(1)

                    f.append(os.path.join(dirpath, filename))
                    found_names.append(filename)
        for i in f:
            self.get_global_enums(i)

    staticmethod(get_global_enums)
    staticmethod(load_all_globals)

    def __init__(self, context, filename, category):
        """This will initialize the module history file. """
        self.context = context
        self.the_name = filename
        self.output_file = os.path.join(self.context['history']['output'],os.path.basename(filename))
        self.the_dict = dict()
        self.module = IndexTracker(1)

        self.load_all_globals()
        self.category = category
        self.the_dict[category] = enum_tracker_int(self.module, category)
        self.the_dict[self.GLOBAL_SECTION] = enum_tracker_int(
            self.GLOBAL_enums.indexer, self.GLOBAL_SECTION)

        try:
            with open(self.the_name, "r") as the_file:
                ix_reader = IndexFile(the_file)
                while True:
                    d = ix_reader.get()
                    if not 'name' in d:
                        break
                    if not d['name'] in self.the_dict:
                        self.the_dict[d['name']] = enum_tracker_int(
                            IndexTracker(None), d['name'])
    
                    self.the_dict[d['name']].setup(d['list'], must_be_unique=True)
        except:
            pass


    def get_global(self, name):
        return self.the_dict[self.GLOBAL_SECTION].get_value(name, None)

    def get_enum(self, name, requested, parent=None):
        if parent is None:
            parent = self.category
        else:
            if parent not in self.the_dict:
                self.the_dict[parent] = enum_tracker_int(
                    IndexTracker(None), parent)

        res = self.the_dict[parent].get_value(name, requested)
        if parent == self.category:
            res += (self.get_global(self.category) << 16)

        return res

    def write(self):
        self.the_name = self.output_file
        print "Writing history to " + self.the_name
        with open(self.the_name, "w") as f:
            f.write("# writing " + self.the_name + "\n")
            writer = IndexFile(f)
            for k in sorted(self.the_dict.keys()):
                writer.write(self.the_dict[k])


def init(context, file, category):
    h = history(context, file, category)
    return h


def close(hist):
    hist.write()

if __name__ == '__main__':
    hf = history(sys.argv[1])
