#!/usr/bin/env python

import re, os, glob, time, sys, binascii, optparse, random

def usageStr():
    usage = ""
    usage += "usage: gen-yagl-calls.py [inputs] [api] [target-guard] [target-header-include-files] [target-source-include-files] [target-output-basename] [host-guard] [host-symbol-prefix] [host-source-include-files] [host-output-basename]"
    return usage

def parseInputFileLine(line):
    entry = {}
    if (line[0] == "@"):
        entry["force_sync"] = True
        line = line.replace("@", "")
    else:
        entry["force_sync"] = False
    tmp = re.match(r"([^\s]+)\s+([^\s]+)\s*\(([^\)]+)?\)", line)
    entry["ret"] = tmp.group(1)
    entry["name"] = tmp.group(2)
    entry["have_ptrs"] = False
    if (tmp.group(3) == None):
        args = []
    else:
        args = tmp.group(3).strip().split(",")
    in_args = []
    out_args = []
    all_args = []
    for arg in args:
        arg = arg.strip()
        tmp = re.match(r"(?:(const)\s+)?([^\s\*\&\^]+)\s*([\*\&\^]+)?\s*(.+)", arg)
        is_out = (tmp.group(1) == "const")

        arg = {}
        arg["name"] = tmp.group(4)
        arg["type"] = tmp.group(2)
        arg["array"] = 0
        arg["va"] = False
        arg["const"] = is_out

        if (tmp.group(3) == "&"):
            if is_out:
                print "const & not allowed"
                sys.exit(1)
        elif (tmp.group(3) == "*"):
            arg["array"] = 1
            entry["have_ptrs"] = True
        elif (tmp.group(3) == "^"):
            arg["va"] = True
            is_out = True
        elif (tmp.group(3) == "^&"):
            arg["va"] = True
            arg["type"] = arg["type"] + " *"
            is_out = False
        elif (tmp.group(3) == None):
            if is_out:
                print "const without array qualifier not allowed"
                sys.exit(1)
            is_out = True
        else:
            print "bad array qualifier - " + tmp.group(3)
            sys.exit(1)
        if is_out:
            out_args += [arg]
        else:
            in_args += [arg]
        arg["out"] = is_out
        arg["in"] = not is_out
        all_args += [arg]
    entry["in_args"] = in_args
    entry["out_args"] = out_args
    entry["args"] = all_args
    return entry

def processInputFiles(input_files):
    lines = []
    for input_file in input_files:
        f = open(input_file, "r")
        text = f.read()
        f.close()
        lines += text.splitlines()
    entries = []
    entry_id = 1
    for line in lines:
        line = line.strip();
        if ((len(line) == 0) or (line[0] == "#")):
            continue
        entry = parseInputFileLine(line)
        entry["id"] = entry_id
        entries += [entry]
        entry_id += 1
    return entries

def argCType(arg, target):
    if arg["out"]:
        if arg["array"] == 1:
            return "const " + arg["type"] + " *"
        elif arg["va"]:
            if target:
                if arg["const"]:
                    return "const " + arg["type"] + " *"
                else:
                    return arg["type"] + " *"
            else:
                return "target_ulong "
        else:
            return arg["type"] + " "
    else:
        if arg["va"]:
            if target:
                if arg["const"]:
                    return "const " + arg["type"] + "*"
                else:
                    return arg["type"] + "*"
            else:
                return "target_ulong *"
        else:
            if arg["array"] <= 1:
                return arg["type"] + " *"

def argCDecl(arg, target):
    out = argCType(arg, target) + arg["name"]
    if arg["array"] == 1:
        if arg["in"]:
            out += ", int32_t " + arg["name"] + "_maxcount, int32_t *" + arg["name"] + "_count"
        else:
            out += ", int32_t " + arg["name"] + "_count"
    return out

def typeSize(type_str):
    if ((type_str == "void") or (type_str == "GLvoid")):
        return "1"
    else:
        return "sizeof(" + type_str + ")"

def entryMinMaxSize(entry):
    out_max = ""
    num_simple_min = num_simple_max = 0
    for arg in entry["args"]:
        if (arg["array"] == 0):
            num_simple_min += 1
            num_simple_max += 1
            if (arg["in"]):
                num_simple_min += 1
                num_simple_max += 1
        elif (arg["array"] == 1):
            num_simple_min += 2 # data_ptr + count
            if arg["in"]:
                out_max += " + yagl_transport_array_size(" + arg["name"] + ", " + arg["name"] + "_maxcount, " + typeSize(arg["type"]) + ")"
            else:
                out_max += " + yagl_transport_array_size(" + arg["name"] + ", " + arg["name"] + "_count, " + typeSize(arg["type"]) + ")"
    if (entry["ret"] != "void"):
        num_simple_min += 2
        num_simple_max += 2
    out_min = str(num_simple_min) + " * 8"
    out_max = str(num_simple_max) + " * 8" + out_max;
    return out_min, out_max;

def generateTargetHeader(output_file, guard, includes, entries):
    out = """/*
 * Generated by gen-yagl-calls.py, do not modify!
 */
#ifndef _%s_H_
#define _%s_H_

#include "yagl_export.h"
""" % (guard, guard)

    for include in includes:
        out += "#include \"" + include + "\"\n";

    out += "\n"

    for entry in entries:
        out += """/*
 * %s wrapper. id = %d
 */
""" % (entry["name"], entry["id"])
        out += entry["ret"] + " yagl_host_" + entry["name"] + "("
        first_arg = True
        for arg in entry["args"]:
            if first_arg:
                first_arg = False
            else:
                out += ", "
            out += argCDecl(arg, True)
        out += ");\n\n"

    out += "#endif\n"

    f = open(output_file, "w")
    f.write(out)
    f.close()

def generateTargetSource(output_file, api, includes, entries):
    out = """/*
 * Generated by gen-yagl-calls.py, do not modify!
 */
"""
    for include in includes:
        out += "#include \"" + include + "\"\n";

    out += """#include "yagl_state.h"
#include <assert.h>
"""

    for entry in entries:
        out += """
/*
 * %s wrapper. id = %d
 */
""" % (entry["name"], entry["id"])
        out += entry["ret"] + " yagl_host_" + entry["name"] + "("

        first_arg = True
        for arg in entry["args"]:
            if first_arg:
                first_arg = False
            else:
                out += ", "
            out += argCDecl(arg, True)

        out += ")\n{\n"

        min_size, max_size = entryMinMaxSize(entry)

        out += "    struct yagl_transport *t = yagl_get_transport();\n"

        if (entry["ret"] != "void"):
            out += "    " + entry["ret"] + " retval;\n\n"
        else:
            out += "\n"

        out += "    yagl_transport_begin(t, " + api + ", " + str(entry["id"]) + ", " + min_size + ", " + max_size + ");\n"

        for arg in entry["out_args"]:
            if (arg["array"] == 0):
                if arg["va"]:
                    out += "    yagl_transport_put_out_va(t, " + arg["name"] + ");\n"
                else:
                    out += "    yagl_transport_put_out_" + arg["type"] + "(t, " + arg["name"] + ");\n"
            elif (arg["array"] == 1):
                out += "    yagl_transport_put_out_array(t, " + arg["name"] + ", " + arg["name"] + "_count, " + typeSize(arg["type"]) + ");\n"
        for arg in entry["in_args"]:
            if (arg["array"] == 0):
                if arg["va"]:
                    out += "    yagl_transport_put_in_va(t, (void**)" + arg["name"] + ");\n"
                else:
                    out += "    yagl_transport_put_in_" + arg["type"] + "(t, " + arg["name"] + ");\n"
            elif (arg["array"] == 1):
                out += "    yagl_transport_put_in_array(t, " + arg["name"] + ", " + arg["name"] + "_maxcount, " + arg["name"] + "_count, " + typeSize(arg["type"]) + ");\n"

        if (entry["ret"] != "void"):
            out += "    yagl_transport_put_in_" + entry["ret"] + "(t, &retval);\n"

        if (entry["have_ptrs"]):
            out += "    if (yagl_transport_direct(t)) {\n"
            out += "        do {\n"
            for arg in entry["out_args"]:
                if (arg["array"] == 1):
                    out += "            yagl_transport_probe_read(" + arg["name"] + ", " + arg["name"] + "_count * " + typeSize(arg["type"]) + ");\n"
            for arg in entry["in_args"]:
                if (arg["array"] == 1):
                    out += "            yagl_transport_probe_write(" + arg["name"] + ", " + arg["name"] + "_maxcount * " + typeSize(arg["type"]) + ");\n"
            out += "        } while (!yagl_transport_end(t));\n"
            out += "    } else {\n"
            out += "        yagl_transport_end(t);\n"
            out += "    }\n"
        else:
            out += "    yagl_transport_end(t);\n"

        if (len(entry["in_args"]) == 0) and (entry["ret"] == "void") and entry["force_sync"]:
            out += "    yagl_transport_sync(t);\n"

        if (entry["ret"] != "void"):
            out += "\n    return retval;\n"

        out += "}\n"

    f = open(output_file, "w")
    f.write(out)
    f.close()

def generateHostHeader(output_file, guard, sym_prefix):
    out = """/*
 * Generated by gen-yagl-calls.py, do not modify!
 */
#ifndef _%s_H_
#define _%s_H_

#include "yagl_types.h"

""" % (guard, guard)

    out += "extern const uint32_t " + sym_prefix + "_num_funcs;\n\n"
    out += "extern yagl_api_func " + sym_prefix + "_funcs[];\n\n"

    out += "#endif\n"

    f = open(output_file, "w")
    f.write(out)
    f.close()

def generateHostSource(output_file, sym_prefix, includes, entries):
    out = """/*
 * Generated by gen-yagl-calls.py, do not modify!
 */
"""
    for include in includes:
        out += "#include \"" + include + "\"\n";

    out += """#include "yagl_thread.h"
#include "yagl_process.h"
#include "yagl_log.h"
"""

    for entry in entries:
        out += """
/*
 * %s dispatcher. id = %d
 */
""" % (entry["name"], entry["id"])
        out += "static bool yagl_func_" + entry["name"] + "(struct yagl_transport *t)\n{\n"

        for arg in entry["args"]:
            out += "    " + argCType(arg, False) + arg["name"] + ";\n"
            if (arg["array"] >= 1):
                if arg["in"]:
                    out += "    int32_t " + arg["name"] + "_maxcount;\n"
                    out += "    int32_t *" + arg["name"] + "_count;\n"
                else:
                    out += "    int32_t " + arg["name"] + "_count;\n"

        if (entry["ret"] != "void"):
            out += "    " + entry["ret"] + " *retval;\n"

        for arg in entry["out_args"]:
            if (arg["array"] == 0):
                if arg["va"]:
                    out += "    " + arg["name"] + " = yagl_transport_get_out_va(t);\n"
                else:
                    out += "    " + arg["name"] + " = yagl_transport_get_out_" + arg["type"] + "(t);\n"
            elif (arg["array"] == 1):
                out += "    if (!yagl_transport_get_out_array(t, " + typeSize(arg["type"]) + ", (const void**)&" + arg["name"] + ", &" + arg["name"] + "_count)) {\n"
                out += "        return false;\n"
                out += "    }\n"

        for arg in entry["in_args"]:
            if (arg["array"] == 0):
                out += "    yagl_transport_get_in_arg(t, (void**)&" + arg["name"] + ");\n"
            elif (arg["array"] == 1):
                out += "    if (!yagl_transport_get_in_array(t, " + typeSize(arg["type"]) + ", (void**)&" + arg["name"] + ", &" + arg["name"] + "_maxcount" ", &" + arg["name"] + "_count)) {\n"
                out += "        return false;\n"
                out += "    }\n"

        if (entry["ret"] != "void"):
            out += "    yagl_transport_get_in_arg(t, (void**)&retval);\n"

        out += "    YAGL_LOG_FUNC_ENTER_SPLIT" + str(len(entry["args"])) + "(" + entry["name"]

        for arg in entry["args"]:
            if ((arg["array"] > 0) or arg["in"]):
                if arg["va"]:
                    out += ", target_ulong*"
                else:
                    out += ", void*"
            else:
                if arg["va"]:
                    out += ", target_ulong"
                else:
                    out += ", " + arg["type"]

        for arg in entry["args"]:
            out += ", " + arg["name"]

        out += ");\n"

        for arg in entry["in_args"]:
            if (arg["array"] == 1):
                out += "    *" + arg["name"] + "_count = 0;\n"

        if (entry["ret"] != "void"):
            out += "    *retval = "
        else:
            out += "    (void)"

        out += "yagl_host_" + entry["name"] + "("

        first_arg = True
        for arg in entry["args"]:
            if first_arg:
                first_arg = False
            else:
                out += ", "
            if (arg["array"] == 0):
                out += arg["name"]
            elif (arg["array"] == 1):
                if arg["in"]:
                    out += arg["name"] + ", " + arg["name"] + "_maxcount, " + arg["name"] + "_count"
                else:
                    out += arg["name"] + ", " + arg["name"] + "_count"

        out += ");\n"

        if (entry["ret"] != "void"):
            out += "    YAGL_LOG_FUNC_EXIT_SPLIT(" + entry["ret"] + ", *retval);\n"
        else:
            out += "    YAGL_LOG_FUNC_EXIT(NULL);\n"

        out += "\n    return true;\n"
        out += "}\n"

    out += "\nconst uint32_t " + sym_prefix + "_num_funcs = " + str(len(entries)) + ";\n\n"
    out += "yagl_api_func " + sym_prefix + "_funcs[] = {\n"

    for entry in entries:
        out += "    &yagl_func_" + entry["name"] + ",\n"

    out += "};\n"

    f = open(output_file, "w")
    f.write(out)
    f.close()

def generateHostDecls(output_file, entries):
    out = ""
    for entry in entries:
        out += entry["ret"] + " yagl_host_" + entry["name"] + "("

        first_arg = True
        for arg in entry["args"]:
            if first_arg:
                first_arg = False
            else:
                out += ",\n    "
            out += argCDecl(arg, False)
        if first_arg:
            out += "void"

        out += ");\n"

    f = open(output_file, "w")
    f.write(out)
    f.close()

if __name__ == '__main__':
    parser = optparse.OptionParser(usage = usageStr())
    opts, args = parser.parse_args()

    argc = len(args)
    if argc < 10:
        parser.print_help()
        sys.exit(1)

    input_files = args[0].split(",")
    api = args[1]
    target_guard = args[2]
    target_h_includes = args[3].split(",")
    target_s_includes = args[4].split(",")
    target_header = args[5] + ".h"
    target_source = args[5] + ".c"
    host_guard = args[6]
    host_sym_prefix = args[7]
    host_s_includes = args[8].split(",")
    host_header = args[9] + ".h"
    host_source = args[9] + ".c"
    host_decls = args[9] + ".txt"

    entries = processInputFiles(input_files)
    generateTargetHeader(target_header, target_guard, target_h_includes, entries)
    generateTargetSource(target_source, api, target_s_includes, entries)
    generateHostHeader(host_header, host_guard, host_sym_prefix)
    generateHostSource(host_source, host_sym_prefix, host_s_includes, entries)
    generateHostDecls(host_decls, entries)
