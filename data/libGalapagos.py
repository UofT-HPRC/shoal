import argparse
import os
import re
# import statistics
import numpy as np
import pandas as pd

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt

LATENCY_ITERATION_COUNT = 100000
THROUGHPUT_ITERATION_COUNT = 100000
TESTS = [
    "libGalapagos_busy",
    "libGalapagos_busy_1core",
    "libGalapagos_no_busy", 
    "libGalapagos_no_busy_10k",
    "libGalapagos_no_busy_1core",
    "libGalapagos_no_busy_affinity"
]
FIGURE_DIRS = ["no_busy_affinity"]
PAYLOADS = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1124]

def get_data(path):
    data = {"test_id": [], "wr_mode": [], "test_mode": [], "payload": [], "time": []}
    test_id = "null"
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
                data["test_id"].append(test_id)
                data["wr_mode"].append(wr_mode)
                data["test_mode"].append(test_mode)
                data["payload"].append(payload)
                data["time"].append(time_sec)
    df = pd.DataFrame.from_dict(data).sort_index()

    return df

def analyze(df, path, figure_dir):

    libGalapagos_path = os.path.join(path, "build", "libGalapagos")
    if not os.path.exists(libGalapagos_path):
        os.makedirs(libGalapagos_path)

    test_cases = df.test_id.unique()

    for test in test_cases:
        test_path = os.path.join(libGalapagos_path, figure_dir)
        if not os.path.exists(test_path):
            os.makedirs(test_path)

        df_subset = df[(df["test_id"] == test)]
        wr_modes = df_subset.wr_mode.unique()

        fig, ax = plt.subplots()
        df_latency = df_subset[df_subset["test_mode"] == "latency"]
        if not df_latency.empty:
            for wr_mode in wr_modes:
                df_payloads = df_latency[df_latency["wr_mode"] == wr_mode]
                ax.plot(df_payloads["payload"], df_payloads["time"], ".-", label=wr_mode)
            ax.set_ylabel("Time (us)")
            ax.set_xlabel("Payload (bytes)")
            ax.set_xscale("log", basex=2)
            ax.legend()
            ax.set_title("Average Latency vs Payload Size")

            fig.tight_layout()
            plt.savefig(os.path.join(test_path, "latency_%s.png" % test))
        plt.close()

        fig, ax = plt.subplots()
        df_throughput = df_subset[df_subset["test_mode"] == "throughput_0"]
        if not df_throughput.empty:
            for wr_mode in wr_modes:
                df_payloads = df_throughput[df_throughput["wr_mode"] == wr_mode]
                ax.plot(df_payloads["payload"], df_payloads["time"], ".-", label=wr_mode)
            ax.set_ylabel("Throughput (Mb/s)")
            ax.set_xlabel("Payload (bytes)")
            ax.set_xscale("log", basex=2)
            ax.legend()
            ax.set_title("Throughput 0 vs Payload Size")

            fig.tight_layout()
            plt.savefig(os.path.join(test_path, "throughput_0_%s.png" % test))
        plt.close()

        fig, ax = plt.subplots()
        df_throughput = df_subset[df_subset["test_mode"] == "throughput_1"]
        if not df_throughput.empty:
            for wr_mode in wr_modes:
                df_payloads = df_throughput[df_throughput["wr_mode"] == wr_mode]
                ax.plot(df_payloads["payload"], df_payloads["time"], ".-", label=wr_mode)
            ax.set_ylabel("Throughput (Mb/s)")
            ax.set_xlabel("Payload (bytes)")
            ax.set_xscale("log", basex=2)
            ax.legend()
            ax.set_title("Throughput 1 vs Payload Size")

            fig.tight_layout()
            plt.savefig(os.path.join(test_path, "throughput_1_%s.png" % test))
        plt.close()




if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract timing from the libGalapagos benchmark tests")
    parser.add_argument("--dir", type=str, default="./data", help="Path to directory with data")

    args = parser.parse_args()
    full_path = os.path.abspath(args.dir)
    if os.path.exists(full_path):
        for index, test in enumerate(TESTS):
            data_path = os.path.join(full_path, test + ".txt")
            df = get_data(data_path)
            figure_dir = test.replace("libGalapagos_", "")
            analyze(df, full_path, figure_dir)
    else:
        print("%s does not exist" % args.dir)