import pandas as pd
import statistics
import os
import seaborn as sns

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from benchmark import IMAGE_TYPES

def get_data(path):
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
        data_avg[test_id]["grid_size"] = test_id_split[0]
        data_avg[test_id]["node_count"] = test_id_split[1]
        data_avg[test_id]["iterations"] = test_id_split[2]
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
    print(df_sorted)
    return df_sorted


def analyze(df, path, suffix):
    # labels = [0]
    # labels.extend([2**x * 8 for x in range(PAYLOAD_MIN, PAYLOAD_MAX)])
    # if include_same:
    #     hue_order = ["sw-sw-same-optimized", "sw-sw-diff-optimized", "sw-hw-optimized", 
    #         "hw-sw-optimized", "hw-hw-same-optimized", "hw-hw-diff"]
    #     legend_labels = ["SW-SW (same)", "SW-SW (diff)", "SW-HW", 
    #         "HW-SW", "HW-HW (same)", "HW-HW (diff)"]
    # else:
    #     hue_order = ["sw-sw-diff-optimized", "sw-hw-optimized", 
    #         "hw-sw-optimized", "hw-hw-diff"]
    #     legend_labels = ["SW-SW (diff)", "SW-HW", 
    #         "HW-SW", "HW-HW (diff)"]
    df_subset = df[df["grid_size"] == 4096]

    # x = np.arange(len(labels))
    fig, ax = plt.subplots()
    sns.barplot(x="nodes", y="total_time", hue='node_count', data=df_subset, ax=ax)
    ax.set_ylabel("Time (s)")
    ax.set_xlabel("Grid Size")
    # ax.set_xticks(x)
    # ax.set_xticklabels(labels)
    # handles, _labels = ax.get_legend_handles_labels()
    # ax.legend(handles, legend_labels)
    
    # ax.legend()
    # plt.xticks(rotation=90)
    # ax.set_title(title)

    fig.tight_layout()
    figure_path = os.path.join(figure_dir, suffix)
    for image_type in IMAGE_TYPES:
        plt.savefig(figure_path + "." + image_type)
    plt.close()

if __name__ == "__main__":
    jacobi_data = [
        ("./data/jacobi_shoal_hw_same.txt", "hw"), 
        ("./data/jacobi_shoal_sw.txt", "sw")
    ]
    figure_dir = os.path.join(os.path.abspath("./data"), "build", "jacobi")
    print(figure_dir)
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)

    for data, suffix in jacobi_data:
        path = os.path.abspath(data)
        df = get_data(path)
        analyze(df, figure_dir, suffix)