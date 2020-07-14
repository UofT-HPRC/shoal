from functools import reduce
import xml.etree.ElementTree as ET
import operator
import json

# some string that will not appear in the project hierarchy
RESERVED_NAME = "varun"

# list of the hierarchy to extract from
# HIER_TO_EXTRACT = ["shellTop", "pr_i", "applicationRegion", "GAScore_0"]
HIER_TO_EXTRACT = ["shellTop", "pr_i", "network"]

# how far in the above hiearchy to extract
HIER_DEPTH = 1

# the LUT, FF, BRAM, and DSP counts in the overall FPGA
FPGA_COUNTS = [-1, "dummy", 663360, 1326720, 2160, 5520]

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
    if hier_name.startswith("("):
        return True
    return False

def extract(root):
    util = {}
    curr_hierarchy = []
    skip = False
    prune = (False, 0)

    for qwerty, tablerow in enumerate(root[0][0].findall("tablerow")):
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

    with open("util.json", "w") as f:
        json.dump(util, f)

    return util

def traverse_hier(util, curr_util, curr_hier, max_hier):
    # print(util)
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

    print(new_rows)

    for pruned_row in to_subtract:
        # print(pruned_row)
        for parent_row in pruned_row[0]:
            # print(parent_row)
            differences = new_rows[parent_row][:2].copy()
            # print(differences)
            for a,b in zip(new_rows[parent_row][2:], pruned_row[1][2:]):
                difference = float(a) - float(b)
                differences.append(difference)
            # print(differences)
            new_rows[parent_row] = differences

    print(new_rows)
    # print(to_subtract)

    return new_rows, new_labels


def add_percentages(rows):
    new_rows = []
    curr_parents = []
    for row in rows:
        if row[0] == 0:
            # curr_parents.append(row)
            # new_rows.append(row)
            # continue
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

    # print(keys[-1])

    # for value in dict_generator(hier):
    #     print(value)

    rows = traverse_hier(hier, [], 0, hier_depth)
    rows_trimmed, new_labels = trim_cols(rows, labels)
    # print(rows_trimmed)
    new_rows = add_percentages(rows_trimmed)
    # print(new_labels)
    write_latex(new_rows)
    # print(new_rows)



if __name__ == "__main__":
    filename = "utilization.txt"
    
    tree = ET.parse(filename)
    root = tree.getroot()
    util = extract(root)
    # with open("util.json", "w") as f:
    #     json.dump(get_from_dict(util, HIER_TO_EXTRACT), f)
    print_table(util, HIER_TO_EXTRACT, hier_depth=HIER_DEPTH)
