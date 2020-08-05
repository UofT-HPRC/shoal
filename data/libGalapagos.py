import argparse
import os
import re
# import statistics
import numpy as np
import pandas as pd
import seaborn as sns
import pickle
import statistics

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

from benchmark import IMAGE_TYPES

LATENCY_ITERATION_COUNT = 100000
THROUGHPUT_ITERATION_COUNT = 100000
TESTS = [
    "libGalapagos_busy",
    "libGalapagos_busy_1core",
    "libGalapagos_no_busy", 
    "libGalapagos_no_busy_diff", 
    "libGalapagos_no_busy_10k",
    "libGalapagos_no_busy_1core",
    "libGalapagos_no_busy_affinity"
]
FIGURE_DIRS = ["no_busy_affinity"]
PAYLOADS = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1124]

def get_data(path):
    data = {"unique": {}, "test_id": [], "wr_mode": [], "test_mode": [], "payload": [], "time": {}}
    test_id = "null"
    index = 0
    with open(path, "r") as f:
        for line in f:
            if line.startswith(" ..."):
                test_id = re.compile('\.+').split(line)[1].strip()
            elif line.startswith("timing"):
                timing_data = line.split(",")
                wr_mode = timing_data[1].strip()
                test_mode = timing_data[2].strip()
                payload = int(timing_data[3].strip())*8 # bytes
                time_sec = float(timing_data[4].strip()) # sec or Mb/s
                if test_mode == "latency":
                    time_sec = time_sec * 1E6 # convert to us
                unique_id = test_id + wr_mode + test_mode + str(payload)
                if unique_id not in data["unique"]:
                    data["unique"][unique_id] = index
                    data["test_id"].append(test_id)
                    data["wr_mode"].append(wr_mode)
                    data["test_mode"].append(test_mode)
                    data["payload"].append(payload)
                    data["time"][unique_id] = [time_sec]
                    index += 1
                else:
                    data["time"][unique_id].append(time_sec)
    
    times = [0]*index
    for key, value in data["time"].items():
        avg = statistics.mean(value)
        idx = data["unique"][key]
        times[idx] = avg
    del data["unique"]
    data["time"] = times
    # print(data)
    df = pd.DataFrame.from_dict(data).sort_index()

    return df

def analyze(df, path, figure_dir):
    labels = [2**x * 8 for x in range(0, 10)]
    labels.append(8992)
    x = np.arange(len(labels))
    

    libGalapagos_path = os.path.join(path, "build", "libGalapagos")
    if not os.path.exists(libGalapagos_path):
        os.makedirs(libGalapagos_path)

    test_cases = df.test_id.unique()

    for test in test_cases:
        test_path = os.path.join(libGalapagos_path, figure_dir)
        if not os.path.exists(test_path):
            os.makedirs(test_path)

        df_subset = df[(df["test_id"] == test)]
        # wr_modes = df_subset.wr_mode.unique()
        # hue_order = ["packet-packet", "packet-packet_mem", "packet_malloc-packet", "packet_malloc-packet_mem"]
        # legend_labels = ["packet (R), packet (W)", "packet (R), packet_mem (W)", "packet_malloc (R), packet (W)", "packet_malloc (R), packet_mem (W)"]

        fig, ax = plt.subplots()
        df_latency = df_subset[df_subset["test_mode"] == "latency"]
        if not df_latency.empty:
            markers = ["o","s","^","v"]*3
            dashes = [(5, 1)]*4 + [(5, 0)]*4 + [(5, 5)]*4
            sns.lineplot(x="payload", y="time", hue='wr_mode', data=df_latency, ax=ax, markers=markers, dashes=dashes, style="wr_mode")
            ax.set_ylabel("Time (us)")
            ax.set_xlabel("Payload Size (bytes)")
            ax.set_xticks(x)
            ax.set_xticklabels(labels)
            ax.set_xscale("log", basex=2)
            ax.xaxis.set_major_formatter(ScalarFormatter())
            handles, _labels = ax.get_legend_handles_labels()
            custom_labels = []
            for label in _labels[1:]:
                label_split = label.split("-")
                custom_labels.append(label_split[0] + " (R) | " + label_split[1] + " (W)")
            ax.legend(handles[1:], custom_labels)
            # ax.legend()
            # # ax.set_title("Average Latency vs Payload Size")

            fig.tight_layout()
            for image_type in IMAGE_TYPES:
                plt.savefig(os.path.join(test_path, f"latency_{test}.{image_type}"))
        plt.close()

        hue_order = ["packet-packet", "packet-packet_mem", "packet_malloc-packet", "packet_malloc-packet_mem"]
        legend_labels = ["packet (R) | packet (W)", "packet (R) | packet_mem (W)", "packet_malloc (R) | packet (W)", "packet_malloc (R) | packet_mem (W)"]

        for test_mode in ["throughput_0", "throughput_1"]:
            fig, ax = plt.subplots()
            df_throughput = df_subset[df_subset["test_mode"] == test_mode]
            if not df_throughput.empty:
                # for wr_mode in wr_modes:
                #     df_payloads = df_throughput[df_throughput["wr_mode"] == wr_mode]
                #     ax.plot(df_payloads["payload"], df_payloads["time"], ".-", label=wr_mode)
                sns.lineplot(x="payload", y="time", hue='wr_mode', data=df_throughput, hue_order=hue_order, ax=ax, marker=".")
                ax.set_ylabel("Throughput (Mb/s)")
                ax.set_xlabel("Payload (bytes)")
                # ax.set_xscale("log", basex=2)
                # ax.set_xticks(x)
                ax.set_xticklabels(labels)
                # ax.legend()
                ax.set_xscale("log", basex=2)
                ax.xaxis.set_major_formatter(ScalarFormatter())
                handles, _labels = ax.get_legend_handles_labels()
                ax.legend(handles[1:], legend_labels)
                # ax.set_title("Throughput 0 vs Payload Size")

                fig.tight_layout()
                for image_type in IMAGE_TYPES:
                    plt.savefig(os.path.join(test_path, f"{test_mode}_{test}.{image_type}"))
            plt.close()

def cross_analyze(data, path):
    figure_path = os.path.join(path, "build", "libGalapagos", "cross")
    if not os.path.exists(figure_path):
        os.makedirs(figure_path)

    for test_mode in ["throughput_0", "throughput_1"]:
        labels = [x * 8 for x in PAYLOADS]
        hue_order = ["no_busy_1core", "busy_1core", "no_busy_affinity", "no_busy", "busy"]
        legend_labels = ["No busy loop (1 core)", "With busy loop (1 core)", "No busy loop (with core affinity)", "No busy loop", "With busy loop"]

        # x = np.arange(len(labels))
        # width = 0.20

        fig, ax = plt.subplots()
        df_sorted = data.sort_values(["test", "payload"])
        df_subset = df_sorted[
            (df_sorted["test_id"] == "reply") & 
            (df_sorted["wr_mode"] == "packet_malloc-packet_mem") &
            (df_sorted["test_mode"] == test_mode) &
            (df_sorted["test"] != "no_busy_10k") 
        ]
        sns.lineplot(x="payload", y="time", hue="test", data=df_subset, marker="o", ax=ax, hue_order=hue_order)
        ax.set_ylabel("Throughput (Mb/s)")
        ax.set_xlabel("Payload Size (bytes)")
        # ax.set_xticks(x)
        ax.set_xticklabels(labels)
        ax.set_xscale("log", basex=2)
        ax.xaxis.set_major_formatter(ScalarFormatter())
        handles, _labels = ax.get_legend_handles_labels()
        ax.legend(handles[1:], legend_labels)
        # ax.legend()
        # plt.xticks(rotation=90)
        # ax.set_title("Throughput vs Payload Size")

        fig.tight_layout()
        for image_type in IMAGE_TYPES:
            plt.savefig(os.path.join(figure_path, f"cross_{test_mode}.{image_type}"))
        plt.close()

    labels = [2**x * 8 for x in range(0, 10)]
    labels.append(8992)
    x = np.arange(len(labels))
    hue_order = ["throughput_0-no_busy", "throughput_1-no_busy", "throughput_0-no_busy_diff", "throughput_1-no_busy_diff"]
    legend_labels = ["Non-Blocking (one node)", "Blocking (one node)", "Non-Blocking (two nodes)", "Blocking (two nodes)"]

    fig, ax = plt.subplots()
    df_sorted = data.sort_values(["test", "payload"])
    df_subset = df_sorted[
        (df_sorted["test_id"] == "reply") & 
        (df_sorted["wr_mode"] == "packet_malloc-packet_mem") &
        (df_sorted["test_mode"] != "latency") &
        ((df_sorted["test"] == "no_busy") |
        (df_sorted["test"] == "no_busy_diff"))
    ]
    df_subset["id"] = df_subset["test_mode"] + "-" + df_subset["test"]
    # print(df_subset)
    sns.lineplot(x="payload", y="time", hue='id', data=df_subset, ax=ax, hue_order=hue_order, marker=".")
    ax.set_ylabel("Throughput (Mb/s)")
    ax.set_xlabel("Payload (bytes)")
    ax.set_xticklabels(labels)
    ax.set_xscale("log", basex=2)
    ax.xaxis.set_major_formatter(ScalarFormatter())
    handles, _labels = ax.get_legend_handles_labels()
    ax.legend(handles[1:], legend_labels)
    # ax.set_title("Throughput 0 vs Payload Size")

    fig.tight_layout()
    for image_type in IMAGE_TYPES:
        plt.savefig(os.path.join(figure_path, f"cross_throughput_reply.{image_type}"))
    plt.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract timing from the libGalapagos benchmark tests")
    parser.add_argument("--dir", type=str, default="./data", help="Path to directory with data")

    args = parser.parse_args()
    full_path = os.path.abspath(args.dir)
    data = None
    if os.path.exists(full_path):
        for index, test in enumerate(TESTS):
            data_path = os.path.join(full_path, test + ".txt")
            df = get_data(data_path)
            figure_dir = test.replace("libGalapagos_", "")
            df["test"] = figure_dir
            # data[figure_dir] = df
            if data is None:
                data = df
            else:
                data = pd.concat([data, df], ignore_index=True)
            # analyze(df, full_path, figure_dir)

        with open("libGalapagos.pickle", "wb") as f:
            pickle.dump(data, f)

        cross_analyze(data, full_path)
    else:
        print("%s does not exist" % args.dir)