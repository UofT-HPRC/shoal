import pandas as pd
import statistics
import os
import seaborn as sns

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# plt.rc("font", family="serif", serif="cmr10")
from benchmark import fonts
plt.rc("font", **fonts)

from benchmark import IMAGE_TYPES

def get_data(path, suffix):
    data = {}
    test_id = ""
    with open(path, "r") as f:
        for line in f:
            if line.startswith("Grid_size: "):
                grid_size = int(line.split(" ")[1].strip())
                test_id = str(grid_size)
            elif line.startswith("Node_count: "):
                try:
                    node_count = int(line.split(" ")[1].strip())
                except Exception as e:
                    print(line)
                    exit(0)
                test_id = test_id + "-" + str(node_count)
            elif line.startswith("Iteration_count: "):
                iteration_count = int(line.split(" ")[1].strip())
                test_id = test_id + "-" + str(iteration_count)
            elif line.startswith("Nodes: "):
                nodes = int(line.split(" ")[1].strip())
                test_id = test_id + "-" + str(nodes)
            elif line.startswith("Cumulative"):
                line_split = line.split(" ")
                time = float(line_split[3].strip())
                
                time_id = line_split[1].strip()
                if test_id not in data:
                    data[test_id] = {}
                if time_id not in data[test_id]:
                    data[test_id][time_id] = [time]
                else:
                    data[test_id][time_id].append(time)

    data_avg = {}
    for test_id, _data in data.items():
        data_avg[test_id] = {}
        test_id_split = test_id.split("-")
        data_avg[test_id]["grid_size"] = int(test_id_split[0])
        data_avg[test_id]["node_count"] = int(test_id_split[1]) - 1
        data_avg[test_id]["iterations"] = int(test_id_split[2])
        data_avg[test_id]["mode"] = suffix
        try:
            data_avg[test_id]["nodes"] = test_id_split[3]
        except IndexError:
            data_avg[test_id]["nodes"] = 1
        for time_id, time_data in _data.items():
            data_avg[test_id][time_id] = statistics.mean(time_data)
            # if len(time_data) > 1:
            #     data_avg[test_id][time_id + "_dev"] = statistics.stdev(time_data)
            # else:
            #     data_avg[test_id][time_id + "_dev"] = 0.0
    
    # columns = ["grid_size", "node_count", "iterations", "nodes",  "memory_init", "communication", "barrier_1", "computation", "barrier_2"]
    df = pd.DataFrame.from_dict(data_avg, orient='index').sort_index()
    df = df.astype({"grid_size": "int", "node_count": "int", "iterations": "int", "nodes": "int"})
    df["total_time"] = df["communication"] + df["barrier_1"] + df["computation"] + df["barrier_2"]
    df_sorted = df.sort_values(["nodes", "node_count", "grid_size"])
    df_sorted = df_sorted.reset_index(drop=True)
    # print(df_sorted)
    return df_sorted


def mk_groups(data):
    try:
        newdata = data.items()
    except:
        return

    thisgroup = []
    groups = []
    for key, value in newdata:
        newgroups = mk_groups(value)
        if newgroups is None:
            thisgroup.append((key, value))
        else:
            thisgroup.append((key, len(newgroups[-1])))
            if groups:
                groups = [g + n for n, g in zip(newgroups, groups)]
            else:
                groups = newgroups
    return [thisgroup] + groups

def add_line(ax, xpos, ypos):
    line = plt.Line2D([xpos, xpos], [ypos + .1, ypos],
                      transform=ax.transAxes, color='black', linewidth=1)
    line.set_clip_on(False)
    ax.add_line(line)

def label_group_bar(ax, data):
    groups = mk_groups(data)
    print(groups)
    xy = groups.pop()
    x, y = zip(*xy)
    ly = len(y)
    xticks = range(1, ly + 1)

    # ax.bar(xticks, y, align='center')
    ax.set_xticks(xticks)
    ax.set_xticklabels(x)
    ax.set_xlim(.5, ly + .5)
    ax.yaxis.grid(False)

    scale = 1. / ly
    for pos in range(ly + 1):
        add_line(ax, pos * scale, -.1)
    ypos = -.2
    while groups:
        group = groups.pop()
        pos = 0
        for label, rpos in group:
            lxpos = (pos + .5 * rpos) * scale
            ax.text(lxpos, ypos, label, ha='center', transform=ax.transAxes)
            add_line(ax, pos * scale, ypos)
            pos += rpos
        add_line(ax, pos * scale, ypos)
        ypos -= .1

def analyze(df, path):
    def add_label(row):
        return str(row["mode"]).upper() + " - " + str(row["nodes"]) + " Nodes (" + str(row["node_count"]) + " kernels)"

    df_subset = df[df["mode"] == "sw"]

    fig, ax = plt.subplots()
    sns.barplot(x="grid_size", y="total_time", hue='node_count', data=df_subset, ax=ax)
    ax.set_ylabel("Time (s)")
    ax.set_xlabel("Grid Size")
    ax.set_yscale("log")
    ticks_x = ticker.FuncFormatter(lambda x, pos: str(x))
    ax.yaxis.set_major_formatter(ticks_x)
    ax.legend(title="Kernels")

    fig.tight_layout()
    figure_path = os.path.join(figure_dir, "sw")
    for image_type in IMAGE_TYPES:
        plt.savefig(figure_path + "." + image_type)
    plt.close()

    df_subset = df[
        (df["node_count"] != 1) & 
        (df["grid_size"] == 4096)
    ]
    df_subset["label"] = df_subset.apply(add_label, axis=1)

    # print(df_subset)

    fig, ax = plt.subplots()

    labels = {
        "Hardware": {
            '1 Node': {
                '8': 0,
            },
            '2 Nodes': {
                '8': 0,
                '16': 0
            },
            '4 Nodes': {
                '16': 0
            }
        },
        "Software": {
            '1 Node': {
                '8': 0,
                '16': 0
            }
        },
    }

    label_group_bar(ax, labels)

    sns.barplot(x="label", y="total_time", data=df_subset, ax=ax)
    ax.set_ylabel("Time (s)")
    # ax.set_xlabel("Total Number of Kernels     ")
    ax.set_xlabel("")
    ax.set_xticklabels(["8 kernels", "8 kernels", "16 kernels", "16 kernels", "8 kernels", "16 kernels"])

    # ax2 = ax.twiny()
    # offset = 0, -25
    # new_axisline = ax2.get_grid_helper().new_fixed_axis
    # ax2.axis["bottom"] = new_axisline(loc="bottom", axes=ax2, offset=offset)
    # ax2.axis["top"].set_visible(False)
    # ax2.set_axticks([0.0, 0.6, 1.0])
    # ax2.xaxis.set_major_formatter(ticker.NullFormatter())
    # ax2.xaxis.set_minor_locator(ticker.FixedLocator([0.3, 0.8]))
    # ax2.xaxis.set_minor_formatter(ticker.FixedFormatter(["mammel", "reptile"]))

    # plt.xticks(rotation=90)
    # ax.set_yscale("log")
    # ax.legend()

    fig.tight_layout()
    figure_path = os.path.join(figure_dir, "hw")
    for image_type in IMAGE_TYPES:
        plt.savefig(figure_path + "." + image_type)
    plt.close()

if __name__ == "__main__":
    jacobi_data = [
        ("./data/jacobi_shoal_hw_same.txt", "hw"), 
        ("./data/jacobi_shoal_sw.txt", "sw")
    ]
    figure_dir = os.path.join(os.path.abspath("./data"), "build", "jacobi")
    # print(figure_dir)
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)

    df = None
    for data, suffix in jacobi_data:
        path = os.path.abspath(data)
        if df is None:
            df = get_data(path, suffix)
        else:
            df = pd.concat([df, get_data(path, suffix)])
    
    analyze(df, figure_dir)