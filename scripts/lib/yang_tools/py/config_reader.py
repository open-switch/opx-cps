import sys
import os
from subprocess import call
import subprocess
import yin_parser
import yin_utils

import object_history

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print(
            "Missing arguements... " +
            sys.argv[
                0] +
            " [cfg file] [tmp path] [out path]")
        sys.exit(1)

    cfg_file = sys.argv[1]
    tmp_path = sys.argv[2]
    out_path = sys.argv[3]

    relative_path = os.path.dirname(cfg_file)

    f = open(cfg_file)

    model_list_name = yin_utils.string_to_c_formatted_name(
        "cps_api_category") + "_t"
    model_list = list()

    for l in f:
        model_file = l.split("\n")[0]
        yang_model = model_file.split(".yang")[0]

        model_cps_name = yang_model
        if model_cps_name.find("dell-") != -1:
            model_cps_name = model_cps_name[
                model_cps_name.find("dell-") + len("dell-"):]

        model_list.append(
            yin_utils.string_to_c_formatted_name(
                "cps_api_cat_" + model_cps_name))
        yin_file = tmp_path + "/" + yang_model + ".yin"
        history_file = relative_path + "/" + yang_model + ".hist"
        header_file = out_path + "/" + yang_model + ".h"

        model_file = relative_path + "/" + model_file
        p = subprocess.Popen(
            ["pyang",
             "-f",
             "yin",
             model_file],
            stdout=subprocess.PIPE)
        of = open(yin_file, "w")
        stdout, stderr = p.communicate()
        for out in stdout:
            of.write(out)
        of.close()
        p.wait()

        old_of = sys.stdout
        sys.stdout = open(header_file, "w")

        yin_parser.process(yin_file, history_file)
        sys.stdout.close()
        sys.stdout = old_of

    object_history.init(cfg_file + ".hist")

    name = os.path.basename(cfg_file)
    if name.rfind(".") != -1:
        name = name[:name.rfind(".")]

    f = open(out_path + "/" + name + ".h", "w")

    yin_utils.header_file_open(cfg_file, name, f)

    f.write("typedef enum " + model_list_name + " { \n")
    en = object_history.get().get_enum(model_list_name)
    for entry in model_list:
        f.write("    " + entry + " = " + str(en.get_value(entry)) + ", \n")

    f.write("} " + model_list_name + "; \n")
    yin_utils.header_file_close(f)

    f.close()

    object_history.close()
