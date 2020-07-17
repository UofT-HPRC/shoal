# Usage:
#   In Vivado, open implementation, and run 
#     "report_utilization -file utilization.txt -format xml -hierarchical" 
#   The filename/path can be whatever, just remember it
#   
#   Then, edit the global options below (variables in all CAPS) as needed
#   and run this script.
#
#   It prints the body of the table to stdout (add the headings etc. yourself)
#   This script prunes any ILAs that are found in the hierarchy and updates the
#   resource usage accordingly. For now, it's only exporting LUT, FF, BRAM and 
#   DSP counts though I'm sure it could be improved to make this more configurable.
#
#   Tested with Vivado 2018.1 and Python 3.7

from functools import reduce
import xml.etree.ElementTree as ET
import operator

# file containing utilization data
FILENAME = "utilization.txt"

# some string that will not appear in the project hierarchy
RESERVED_NAME = "varun"

# list of the hierarchy to extract from
HIER_TO_EXTRACT = ["shellTop", "pr_i", "applicationRegion"]
# HIER_TO_EXTRACT = ["shellTop", "shell_i"]

# how far in the above hiearchy to extract
HIER_DEPTH = 1

# the LUT, FF, BRAM, and DSP counts in the overall FPGA.
# Leave the first two values alone
FPGA_COUNTS = [-1, "dummy", 663360, 1326720, 2160, 5520]

# calculate percent usage of a block relative to its parent.
# The top level is compared against FPGA_COUNTS
ADD_PERCENTAGES = True

def get_hier(name):
    space_per_hier = 2
    hier_name = name.lstrip(" ")
    hier_level = int((len(name) - len(hier_name)) / space_per_hier)
    return hier_name, hier_level # for hier, plus children

# https://stackoverflow.com/a/14692747
def get_from_dict(data_dict, keys):
    return reduce(operator.getitem, keys, data_dict)

def set_in_dict(data_dict, keys, value):
    get_from_dict(data_dict, keys[:-1])[keys[-1]] = value

def append_in_dict(data_dict, keys, value):
    get_from_dict(data_dict, keys[:-1])[keys[-1]].append(value)

def should_skip(hier_name):
    # not sure why these entities exist in the XML. For now, just remove them
    if hier_name.startswith("("):
        return True
    return False

def extract(root):
    util = {}
    curr_hierarchy = []
    skip = False
    prune = (False, 0)

    for tablerow in root[0][0].findall("tablerow"):
        for index, subelem in enumerate(tablerow):
            if index == 0:
                hier_name, hier_level = get_hier(subelem.get("contents"))
                skip = should_skip(hier_name)
                if not skip:
                    try:
                        _dummy = curr_hierarchy[hier_level]
                    except IndexError as e:
                        curr_hierarchy.append(hier_name)
                    else:
                        if len(curr_hierarchy) > hier_level + 1:
                            curr_hierarchy = curr_hierarchy[:hier_level + 1]
                        curr_hierarchy[hier_level] = hier_name
                    finally:
                        set_in_dict(util, curr_hierarchy, {RESERVED_NAME: []})

            if not skip:
                tmp = curr_hierarchy.copy()
                tmp.append(RESERVED_NAME)
                append_in_dict(util, tmp, subelem.get("contents"))

    return util

def traverse_hier(util, curr_util, curr_hier, max_hier):
    hier_name = util[RESERVED_NAME][0].lstrip()
    hier_name = "  " * curr_hier + hier_name
    row = [curr_hier, hier_name]
    row.extend(util[RESERVED_NAME][2:])
    curr_util.append(row)
    if curr_hier < max_hier:
        for key, value in util.items():
            if key == RESERVED_NAME:
                continue
            else:
                curr_util = traverse_hier(value, curr_util, curr_hier+1, max_hier)
    return curr_util

def should_prune(prune, row):
    hier_name = row[1]
    hier_level = row[0]

    if not prune[0]:
        if "_ila_" in hier_name:
            return (True, hier_level), True
        return (False, hier_level), False
    else:
        if hier_level <= prune[1]:
            return (False, hier_level), False
        return (True, hier_level), False

    # return (False, hier_level), False

def trim_cols(rows, labels):
    LUTS = 2
    FFS = 6
    RAM_36 = 7
    RAM_18 = 8
    DSPS = 10

    new_rows = []
    new_labels = ["LUTs", "FFs", "BRAMs", "DSPs"]
    prune = (False, 0)
    start_prune = False
    to_subtract = []

    for index, row in enumerate(rows):
        prune, start_prune = should_prune(prune, row)
    
        new_row = row[:2]
        new_row.append(row[LUTS])
        new_row.append(row[FFS])
        new_row.append(float(row[RAM_36]) + float(row[RAM_18])/2)
        new_row.append(row[DSPS])
        if not prune[0]:
            new_rows.append(new_row)
        if start_prune:
            index_to_start = row[0] - 1
            save = []
            for index_prime, row_prime in enumerate(reversed(rows[:index+1])):
                if row_prime[0] == index_to_start:
                    save.append(index - index_prime)
                    index_to_start -= 1
                if index_to_start < 0:
                    break
            to_subtract.append((save, new_row))

    for pruned_row in to_subtract:
        for parent_row in pruned_row[0]:
            differences = new_rows[parent_row][:2].copy()
            for a,b in zip(new_rows[parent_row][2:], pruned_row[1][2:]):
                difference = float(a) - float(b)
                differences.append(difference)
            new_rows[parent_row] = differences

    return new_rows, new_labels


def add_percentages(rows):
    new_rows = []
    curr_parents = []
    for row in rows:
        if row[0] == 0:
            curr_parent = FPGA_COUNTS
        else:
            parent_i = curr_parents[-1][0]
            to_del = 0
            while parent_i != row[0] - 1:
                to_del += 1
                parent_i = curr_parents[-1-to_del][0]
            if to_del > 0:
                del curr_parents[-to_del:]
            
            curr_parent = curr_parents[-1]
        percents = []
        for a,b in zip(curr_parent[2:], row[2:]):
            try:
                quotient = float(b) / float(a) * 100
            except ZeroDivisionError as e:
                quotient = 0.0
            percents.append(quotient)

        new_row = [row[0], row[1]]
        for a,b in zip(row[2:], percents):
            new_row.append(a)
            new_row.append(b)
        new_rows.append(new_row)

        if row[0] > curr_parent[0]:
            curr_parents.append(row)

    return new_rows

def write_latex(rows):
    for row in rows:
        latex_row = f"\\tableindent[{row[0]}] "
        for col in row[1:]:
            text = str(col).strip()
            text = text.replace("_", "\\_")
            latex_row += text + " & "
        latex_row = latex_row[:-2] + "\\\\"
        print(latex_row)

def print_table(util, keys, hier_depth=1000):
    labels = util["Instance"][RESERVED_NAME]
    hier = get_from_dict(util, HIER_TO_EXTRACT)

    rows = traverse_hier(hier, [], 0, hier_depth)
    rows_trimmed, new_labels = trim_cols(rows, labels)
    if ADD_PERCENTAGES:
        new_rows = add_percentages(rows_trimmed)
    else:
        new_rows = rows_trimmed
    write_latex(new_rows)


if __name__ == "__main__":
    tree = ET.parse(FILENAME)
    root = tree.getroot()
    util = extract(root)
    print_table(util, HIER_TO_EXTRACT, hier_depth=HIER_DEPTH)
