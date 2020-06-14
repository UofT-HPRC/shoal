import argparse
import os
# import statistics
import numpy as np
import pandas as pd
import seaborn as sns

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt

LATENCY_ITERATION_COUNT = 1000
THROUGHPUT_ITERATION_COUNT = 1000
# TESTS = ["hw-hw-same", "sw-sw-same", "sw-hw"]
TESTS = ["hw-hw-same-optimized"]
TRANSPORT = ["tcp"]
PAYLOAD_MIN = 0
PAYLOAD_MAX = 10
THROUGHPUT_TIMES = 2

# payload, label
STRIDED_METADATA = [
    (32, "32B to 4 8B [16B stride]"),
]

# payload
VECTOR_METADATA = [
    (24, "2 [16B, 8B]"),
    (24, "2 [16B, 8B] to 1"),
    (24, "1 [24B] to 2 [16B, 8B]"),
]

def get_data(path, transport, test_type):

    def test_to_cols(test_id):
        # message, type, payload
        test_id_parts = test_id.split("_")
        test_id_parts[0] = test_id_parts[0].replace("-", " ").capitalize().replace("fifo", "FIFO")
        test_id_parts[1] = test_id_parts[1].capitalize()
        return test_id_parts

    data = {}
    test_id = "null"
    with open(path, "r") as f:
        for line in f:
            if line.startswith("test: "):
                if test_id != "null":
                    data[test_id].extend(test_to_cols(test_id))
                    data[test_id].append(transport)
                test_id = line.split(":")[1].strip()
                if test_id not in data:
                    data[test_id] = []
            elif line.startswith("timing: "):
                cycles = int(line.split(":")[1].strip())                  
                # convert cycles (156.25 MHz, 6.4 ns cycles) to ms
                time_ms = cycles * 6.4 / 1E6
                data[test_id].append(time_ms)
        data[test_id].extend(test_to_cols(test_id))
        data[test_id].append(transport)
    df = pd.DataFrame.from_dict(data, orient='index').sort_index()
    if test_type == "latency":
        last_col = LATENCY_ITERATION_COUNT
    else:
        last_col = THROUGHPUT_TIMES
    df = df.rename(columns={last_col: "Test", (last_col+1): "Test_Type", (last_col+2): "Payload", (last_col+3): "Transport"})
    cols = list(df)
    cols.insert(0, cols.pop(cols.index("Transport")))
    cols.insert(0, cols.pop(cols.index("Payload")))
    cols.insert(0, cols.pop(cols.index("Test_Type")))
    cols.insert(0, cols.pop(cols.index("Test")))
    df = df.loc[:, cols]
    df.columns = df.columns.astype(str)

    def reset_short_payload(row):
        if row["Test"] == "Short":
            return 0
        if row["Test"] in ["Vector", "Strided"]:
            return int(row["Payload"])
        return 2**int(row["Payload"]) * 8
    df["Payload"] = df.apply(reset_short_payload, axis=1)
    df["Test"] = pd.Categorical(df["Test"], ["Short", "Medium FIFO", "Medium", "Long FIFO", "Long", "Strided", "Vector"])

    return df


def analyze_latency_iterations(df, path, row_label):
    try:
        row = df.loc[row_label, str(0):str(LATENCY_ITERATION_COUNT-1)]
    except KeyError:
        return
    fig, ax = plt.subplots()
    ax.plot(pd.Series(range(0,LATENCY_ITERATION_COUNT)), row, label="_hidden")
    mean = df.loc[row_label, "mean"]
    fifth_percent = df.loc[row_label, "5th %tile"]
    ninefifth_percent = df.loc[row_label, "95th %tile"]
    median = df.loc[row_label, "median"]
    ax.axhline(mean, label="mean", linestyle="--", color="r")
    ax.axhline(median, label="median", linestyle="--", color="c")
    ax.axhline(fifth_percent, label="5th %tile", linestyle="--", color="g")
    ax.axhline(ninefifth_percent, label="95th %tile", linestyle="--", color="b")
    ax.set_ylabel("Time (ms)")
    ax.set_xlabel("Iteration")
    
    ax.set_title(df.loc[row_label, "Label"] + " " + df.loc[row_label, "Test_Type"])
    ax.legend()

    fig.tight_layout()
    plt.savefig(os.path.join(path, row_label + ".png"))
    plt.close()


def analyze_latency_medians(df, path):
    labels = [2**x * 8 for x in range(PAYLOAD_MIN, PAYLOAD_MAX)]

    x = np.arange(len(labels))
    width = 0.20

    # fig, ax = plt.subplots()
    # df_sorted = df.sort_values(["Payload", "Test"])
    # df_subset = df_sorted[(df_sorted["Test"] == "Medium FIFO")]
    # ax.bar(x - (3*width)/2, df_subset["median"], width, label="Medium FIFO")
    # df_subset = df_sorted[(df_sorted["Test"] == "Medium")]
    # ax.bar(x - width/2, df_subset["median"], width, label="Medium")
    # df_subset = df_sorted[(df_sorted["Test"] == "Long FIFO")]
    # ax.bar(x + width/2, df_subset["median"], width, label="Long FIFO")
    # df_subset = df_sorted[(df_sorted["Test"] == "Long")]
    # ax.bar(x + (3*width/2), df_subset["median"], width, label="Long")
    # ax.set_ylabel("Time (ms)")
    # ax.set_xlabel("Payload Size (bytes)")
    # ax.set_xticks(x)
    # ax.set_xticklabels(labels)
    # ax.legend()
    # # plt.xticks(rotation=90)
    # ax.set_title("Median Latency by Message Types")

    fig, ax = plt.subplots()
    df_sorted = df.sort_values(["Payload", "Test"])
    df_subset = df_sorted[
        (df_sorted["Test"] == "Medium FIFO") | 
        (df_sorted["Test"] == "Medium") |
        (df_sorted["Test"] == "Long FIFO") |
        (df_sorted["Test"] == "Long")
    ]
    # print(df_subset)
    sns.barplot(x="Payload", y='median', hue='Test', data=df_subset, ax=ax, hue_order=["Medium FIFO", "Medium", "Long FIFO", "Long"])
    # ax.bar(x - (3*width)/2, df_subset["median"], width, label="Medium FIFO")
    # df_subset = df_sorted[(df_sorted["Test"] == "Medium")]
    # ax.bar(x - width/2, df_subset["median"], width, label="Medium")
    # df_subset = df_sorted[(df_sorted["Test"] == "Long FIFO")]
    # ax.bar(x + width/2, df_subset["median"], width, label="Long FIFO")
    # df_subset = df_sorted[(df_sorted["Test"] == "Long")]
    # ax.bar(x + (3*width/2), df_subset["median"], width, label="Long")
    ax.set_ylabel("Time (ms)")
    ax.set_xlabel("Payload Size (bytes)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()
    # plt.xticks(rotation=90)
    ax.set_title("Median Latency by Message Types")

    fig.tight_layout()
    plt.savefig(os.path.join(path, "medians_all.png"))
    plt.close()

    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset["median"], ".")
        ax.set_ylabel("Time (ms)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        ax.set_title("Median Latency Time of %s Messages" % test)

        fig.tight_layout()
        plt.savefig(os.path.join(path, "medians_%s.png" % test))
        plt.close()

def analyze_latency_throughput(df, path):
    def get_throughput(row):
        test_id = row["Test"]
        payload = row["Payload"]

        # time is in ms, throughput in Gb/s
        test_time = row["mean"]
        if test_id == "Short":
            return 0
        elif test_id == "Strided":
            return STRIDED_METADATA[row["Payload"]][0]/test_time*1000*8/1E9
        elif test_id == "Vector":
            return VECTOR_METADATA[row["Payload"]][0]/test_time*1000*8/1E9
        else:
            return payload/test_time*1000*8/1E9

    df["payload_throughput"] = df.apply(get_throughput, axis=1)

    labels = [2**x * 8 for x in range(PAYLOAD_MIN, PAYLOAD_MAX)]

    x = np.arange(len(labels))
    width = 0.20

    fig, ax = plt.subplots()
    df_sorted = df.sort_values(["Payload", "Test"])
    df_subset = df_sorted[(df_sorted["Test"] == "Medium FIFO")]
    ax.bar(x - (3*width)/2, df_subset["payload_throughput"], width, label="Medium FIFO")
    df_subset = df_sorted[(df_sorted["Test"] == "Medium")]
    ax.bar(x - width/2, df_subset["payload_throughput"], width, label="Medium")
    df_subset = df_sorted[(df_sorted["Test"] == "Long FIFO")]
    ax.bar(x + width/2, df_subset["payload_throughput"], width, label="Long FIFO")
    df_subset = df_sorted[(df_sorted["Test"] == "Long")]
    ax.bar(x + (3*width/2), df_subset["payload_throughput"], width, label="Long")
    ax.set_ylabel("Throughput (Gb/s)")
    ax.set_xlabel("Payload Size (bytes)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()
    # plt.xticks(rotation=90)
    ax.set_title("Payload Throughput (latency mode) by Message Types")

    fig.tight_layout()
    plt.savefig(os.path.join(path, "payload_throughput_all.png"))
    plt.close()

    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset["payload_throughput"], ".")
        ax.set_ylabel("Throughput (Gb/s)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        ax.set_title("Payload Throughput (latency mode) of %s Messages" % test)

        fig.tight_layout()
        plt.savefig(os.path.join(path, "payload_throughput_%s.png" % test))
        plt.close()


def analyze_one(df, path, test_type):
    def add_label(row):
        test_id = row["Test"]
        payload = row["Payload"]
        if test_id == "Short":
            title = "%s Message" % (test_id)
        elif test_id == "Strided":
            title = "%s Message (%s)" % (test_id, STRIDED_METADATA[row["Payload"]][1])
        elif test_id == "Vector":
            title = "%s Message (%s)" % (test_id, VECTOR_METADATA[row["Payload"]][1])
        else:
            title = "%s Message (%dB payload)" % (test_id, payload)
        return title

    def shorten_label(row):
        test_id = row["Test"]
        if not isinstance(test_id, str):
            print(test_id)
        if test_id == "Short":
            test_initial = "S"
        elif test_id.startswith("Medium"):
            test_initial = "M"
        elif test_id == "Vector":
            test_initial = "Vec"
        elif test_id == "Strided":
            test_initial = "Str"
        else:
            test_initial = "L"
        
        if "FIFO" in test_id:
            test_initial += " F"
        payload =row["Payload"]
        if test_id == "Short":
            test_payload = ""
        elif test_id in ["Vector", "Strided"]:
            test_payload = str(payload)
        else:
            test_payload = " (%dB)" % payload
        return test_initial + test_payload

    # print(df)

    df["Label"] = df.apply(add_label, axis=1)
    df["Label_short"] = df.apply(shorten_label, axis=1)

    if test_type == "latency":
        analyze_latency(df, path)
    else:
        analyze_throughput(df, path)

def analyze_latency(df, path):
    col = df.loc[:, str(0):str(LATENCY_ITERATION_COUNT-1)]
    df["mean"] = col.mean(axis=1)
    df["5th %tile"] = col.apply(np.percentile, args=[5], axis=1)
    df["95th %tile"] = col.apply(np.percentile, args=[95], axis=1)
    df["median"] = col.apply(np.percentile, args=[50], axis=1)
    
    # print(df)
    
    df_rows = []
    df_rows.append({"label": "short_latency_0", "title": "Short Message Latency"})
    for i in range(PAYLOAD_MIN, PAYLOAD_MAX):
        payload = 2**i * 8
        df_rows.append({"label": "medium_latency_" + str(i), "title": "Medium Latency (%d byte payload)" % payload})
        df_rows.append({"label": "medium-fifo_latency_" + str(i), "title": "Medium FIFO Latency (%d byte payload)" % payload})
        df_rows.append({"label": "long_latency_" + str(i), "title": "Long Latency (%d byte payload)" % payload})
        df_rows.append({"label": "long-fifo_latency_" + str(i), "title": "Long FIFO Latency (%d byte payload)" % payload})
    for i in range(len(STRIDED_METADATA)):
        df_rows.append({"label": "strided_latency_" + str(i), "title": STRIDED_METADATA[i][1] + " Latency"})
    for i in range(len(VECTOR_METADATA)):
        df_rows.append({"label": "vector_latency_" + str(i), "title": VECTOR_METADATA[i][1] + " Latency"})

    figure_dir = os.path.join(path, "iterations")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    for row in df_rows:
        analyze_latency_iterations(df, figure_dir, row["label"])
    
    figure_dir = os.path.join(path, "medians")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    analyze_latency_medians(df, figure_dir)

    figure_dir = os.path.join(path, "throughput")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    analyze_latency_throughput(df, figure_dir)

def analyze_throughput_efficiency(df, path, payload_label):
    labels = [2**x * 8 for x in range(PAYLOAD_MIN, PAYLOAD_MAX)]

    x = np.arange(len(labels))
    width = 0.20

    fig, ax = plt.subplots()
    df_sorted = df.sort_values(["Payload", "Test"])
    df_subset = df_sorted[(df_sorted["Test"] == "Medium FIFO")]
    ax.bar(x - (3*width)/2, df_subset[payload_label], width, label="Medium FIFO")
    df_subset = df_sorted[(df_sorted["Test"] == "Medium")]
    ax.bar(x - width/2, df_subset[payload_label], width, label="Medium")
    df_subset = df_sorted[(df_sorted["Test"] == "Long FIFO")]
    ax.bar(x + width/2, df_subset[payload_label], width, label="Long FIFO")
    df_subset = df_sorted[(df_sorted["Test"] == "Long")]
    ax.bar(x + (3*width/2), df_subset[payload_label], width, label="Long")
    ax.set_ylabel("Throughput (Gb/s)")
    ax.set_xlabel("Payload Size (bytes)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()
    # plt.xticks(rotation=90)
    ax.set_title("Payload Throughput by Message Types")

    fig.tight_layout()
    plt.savefig(os.path.join(path, payload_label + "_all.png"))
    plt.close()

    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset[payload_label], ".")
        ax.set_ylabel("Throughput (Gb/s)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        ax.set_title("Payload Throughput of %s Messages" % test)

        fig.tight_layout()
        plt.savefig(os.path.join(path, payload_label + "_%s.png" % test))
        plt.close()

def analyze_throughput_latency(df, path):
    def get_latency(row):
        test_id = row["Test"]
        payload = row["Payload"]

        # test time is in ms
        test_time = row["1"]
        return row["0"]/THROUGHPUT_ITERATION_COUNT

    df["payload_latency"] = df.apply(get_latency, axis=1)
    labels = [2**x * 8 for x in range(PAYLOAD_MIN, PAYLOAD_MAX)]

    x = np.arange(len(labels))
    width = 0.20

    fig, ax = plt.subplots()
    df_sorted = df.sort_values(["Payload", "Test"])
    df_subset = df_sorted[(df_sorted["Test"] == "Medium FIFO")]
    ax.bar(x - (3*width)/2, df_subset["payload_latency"], width, label="Medium FIFO")
    df_subset = df_sorted[(df_sorted["Test"] == "Medium")]
    ax.bar(x - width/2, df_subset["payload_latency"], width, label="Medium")
    df_subset = df_sorted[(df_sorted["Test"] == "Long FIFO")]
    ax.bar(x + width/2, df_subset["payload_latency"], width, label="Long FIFO")
    df_subset = df_sorted[(df_sorted["Test"] == "Long")]
    ax.bar(x + (3*width/2), df_subset["payload_latency"], width, label="Long")
    ax.set_ylabel("Time (ms)")
    ax.set_xlabel("Payload Size (bytes)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()
    # plt.xticks(rotation=90)
    ax.set_title("Average Latency (throughput mode) by Message Types")

    fig.tight_layout()
    plt.savefig(os.path.join(path, "average_all.png"))
    plt.close()

    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset["payload_latency"], ".")
        ax.set_ylabel("Time (ms)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        ax.set_title("Average Latency (throughput mode) Time of %s Messages" % test)

        fig.tight_layout()
        plt.savefig(os.path.join(path, "averages_%s.png" % test))
        plt.close()

def analyze_throughput(df, path):
    def get_throughput(row, label):
        test_id = row["Test"]
        payload = row["Payload"]

        # time is in ms
        test_time = row[label]
        if test_id == "Short":
            return 0
        elif test_id == "Strided":
            return STRIDED_METADATA[row["Payload"]][0]*THROUGHPUT_ITERATION_COUNT/test_time*1000*8/1E9
        elif test_id == "Vector":
            return VECTOR_METADATA[row["Payload"]][0]*THROUGHPUT_ITERATION_COUNT/test_time*1000*8/1E9
        else:
            return payload*THROUGHPUT_ITERATION_COUNT/test_time*1000*8/1E9

    df["payload_throughput_0"] = df.apply(get_throughput, args=("0",), axis=1)
    if THROUGHPUT_TIMES > 1:
        df["payload_throughput_1"] = df.apply(get_throughput, args=("1",), axis=1)

    # print(df)

    figure_dir = os.path.join(path, "efficiency")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    for throughput_instance in range(THROUGHPUT_TIMES):
        label = "payload_throughput_" + str(throughput_instance)
        analyze_throughput_efficiency(df, figure_dir, label)

    figure_dir = os.path.join(path, "latency")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    analyze_throughput_latency(df, figure_dir)

def summarize(data_dir):
    data = {}
    for test in TESTS:
        data[test] = {}
        for transport in TRANSPORT:
            data[test][transport] = {}
            for test_type in ["latency", "throughput"]:
                test_data = "microbenchmarks_" + test + "_" + test_type + "_" + transport + "_raw.txt"
                test_data_path = os.path.join(data_dir, test_data)
                if os.path.exists(test_data_path):
                    df = get_data(test_data_path, transport, test_type)
                    data[test][transport][test_type] = df
                    figure_dir = os.path.join(data_dir, "build", test, transport, test_type)
                    if not os.path.exists(figure_dir):
                        os.makedirs(figure_dir)
                    analyze_one(df, figure_dir, test_type)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract timing from the microbenchmark test")
    parser.add_argument("--dir", type=str, default="./data", help="Path to directory with data")

    args = parser.parse_args()
    full_path = os.path.abspath(args.dir)
    if os.path.exists(full_path):
        summarize(full_path)
    else:
        print("%s does not exist" % args.dir)

