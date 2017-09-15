

__use_standard_copyright = False

__standard_copyright = \
    """

/*
* (c) Copyright 2017 Dell Inc. All Rights Reserved.
*/

"""

__open_source_license = \
    '''
/*
* Copyright (c) 2017 Dell Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License"); you may
* not use this file except in compliance with the License. You may obtain
* a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
* THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
* LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
* FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
* See the Apache Version 2.0 License for specific language governing
* permissions and limitations under the License.
*/
'''


# Create a string that can be used is C programs
def string_to_c_formatted_name(s):
    s = s.replace('-', '_')
    s = s.replace(':', '_')
    return s.upper()


def add_copyright_to_file(stream):
    """
    Add a copyright to the steam
    @src_file the actual file name of the stream
    @stream the file to be written
    """
    if __use_standard_copyright:
        stream.write(__standard_copyright)
        stream.write("/* OPENSOURCELICENSE */" + "\n")
    else:
        stream.write(__open_source_license)


def header_file_open(src_file, mod_name, stream):
    """
    Write the standard C header file constructs to a file
    In the header, the filename will be printed along with a dell copyright
    @src_file the source file name - added to header
    @mod_name this will be used for ifdef/ifndef constructs
    @stream the file stream that will be used for writing
    """
    stream.write("/*Source file name -> %s*/\n" % src_file)

    add_copyright_to_file(stream)

    stream.write(
        "#ifndef " +
        string_to_c_formatted_name(
            mod_name +
            "_H") +
        "\n")
    stream.write(
        "#define " +
        string_to_c_formatted_name(
            mod_name +
            "_H") +
        "\n")
    stream.write("" + "\n")
    stream.write("" + "\n")

    # hmmmm... not sure why cps_api_operation.h needs to be included...
    stream.write("#include \"cps_api_operation.h\"\n")
    stream.write("#include <stdint.h>\n")
    stream.write("#include <stdbool.h>\n")


def header_file_close(stream):
    stream.write("#endif" + "\n")
