import os
import pandas as pd
import numpy as np

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from benchmark import IMAGE_TYPES

# FILE_NAME = "microbenchmarks_sw-sw-same-tmp-2_throughput_tcp_raw.txt"
FILE_NAMES = [
    "libGalapagos_detail_busy.txt",
    "libGalapagos_detail_no_busy.txt",
    "microbenchmarks_sw-sw-same-long-detail_throughput_tcp_raw.txt",
    "microbenchmarks_sw-sw-same-long-detail-no-busy_throughput_tcp_raw.txt",
]
FIGURE_DIRS= [
    "libGalapagos/detail_busy",
    "libGalapagos/detail_no_busy",
    "sw-sw-same/detail_busy",
    "sw-sw-same/detail_no_busy",
]
THROUGHPUT_ITERATION_COUNT = 100000

def main():
    path = os.environ["SHOAL_PATH"] + "/data"
    for file_name, figure_dir in list(zip(FILE_NAMES, FIGURE_DIRS)):
        data_path = path + "/" + file_name
        df = get_data(data_path)
        # print(df)
        figure_path = path + "/build/" + figure_dir
        if not os.path.exists(figure_path):
            os.makedirs(figure_path)
        analyze_iterations(df, figure_path)

def analyze_iterations(data, path):
    for key, value in data.iteritems():
        # if len(value) != THROUGHPUT_ITERATION_COUNT:
        #     print(key, value)
        #     continue
        
        allowed_labels = [
            "kernel_receive_0", "kernel_receive_1", "kernel_write_0", "kernel_write_1",
            "kernel_send_0", "kernel_wait_0", "kernel_send_1", "kernel_wait_1",
            "kernel_send_2_0", "kernel_send_2_1", "kernel_wait_2", "kernel_send_3_0", "kernel_send_3_1", "kernel_wait_3"
        ]
        if key not in allowed_labels:
            continue
        
        fig, ax = plt.subplots()
        value_np = np.array(value)
        ax.plot(pd.Series(range(0,len(value))), value, ".", label="_hidden")
        # mean = df.loc[row_label, "mean"]
        # fifth_percent = df.loc[row_label, "5th %tile"]
        # ninefifth_percent = df.loc[row_label, "95th %tile"]
        # median = df.loc[row_label, "median"]
        ax.axhline(float(np.mean(value_np)), label="mean: " + "%.2E" % float(np.mean(value_np)), linestyle="--", color="r")
        ax.axhline(float(np.median(value_np)), label="median: " + "%.2E" % float(np.median(value_np)), linestyle="--", color="c")
        ax.axhline(float(np.percentile(value_np, 5)), label="5th %tile: " + "%.2E" % float(np.percentile(value_np, 5)), linestyle="--", color="g")
        ax.axhline(float(np.percentile(value_np, 95)), label="95th %tile: " + "%.2E" % float(np.percentile(value_np, 95)), linestyle="--", color="b")
        ax.set_ylabel("Time (ms)")
        ax.set_xlabel("Iteration")
        
        ax.set_title(key)
        ax.legend()

        fig.tight_layout()
        for image_type in IMAGE_TYPES:
            plt.savefig(os.path.join(path, key + "." + image_type))
        plt.close()

def get_data(path):
    data = {}
    test_id = "null"
    with open(path, "r") as f:
        for line in f:
            if line.startswith("test: "):
                test_id = line.split(":")[1].strip()
                print(test_id)
                # if test_id not in data:
                #     data[test_id] = {}
            elif line.startswith("timing:"):
                cycles = int(line.split(":")[1].strip())
                time_s = cycles * 6.4 / 1E9
                gbits = 4096 * THROUGHPUT_ITERATION_COUNT * 8 / 1E9
                print(gbits / time_s)
            elif line.startswith("timing,"):
                print(line)
            elif line.startswith("mem:"):
                print("mem", line.split(":")[1].strip())
            elif line.startswith("kernel_"):
                label = line.split(":")[0].strip()
                try:
                    cycles = int(line.split(":")[1].strip())
                    # convert cycles (156.25 MHz, 6.4 ns cycles) to ms
                    time_ms = cycles * 6.4 / 1E6
                    if label not in data:
                        data[label] = []
                    data[label].append(time_ms)
                except:
                    continue
    # df = pd.DataFrame.from_dict(data).sort_index()
    # if test_type == "latency":
    #     last_col = LATENCY_ITERATION_COUNT
    # else:
    #     last_col = THROUGHPUT_TIMES
    # df = df.rename(columns={last_col: "Test", (last_col+1): "Test_Type", (last_col+2): "Payload", (last_col+3): "Transport"})
    # cols = list(df)
    # cols.insert(0, cols.pop(cols.index("Transport")))
    # cols.insert(0, cols.pop(cols.index("Payload")))
    # cols.insert(0, cols.pop(cols.index("Test_Type")))
    # cols.insert(0, cols.pop(cols.index("Test")))
    # df = df.loc[:, cols]
    # df.columns = df.columns.astype(str)

    # def reset_short_payload(row):
    #     if row["Test"] == "Short":
    #         return 0
    #     if row["Test"] in ["Vector", "Strided"]:
    #         return int(row["Payload"])
    #     return 2**int(row["Payload"]) * 8
    # df["Payload"] = df.apply(reset_short_payload, axis=1)
    # df["Test"] = pd.Categorical(df["Test"], ["Short", "Medium FIFO", "Medium", "Long FIFO", "Long", "Strided", "Vector"])

    return data

if __name__ == "__main__":
    main()