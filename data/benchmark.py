import argparse
import os
import pickle
import numpy as np
import pandas as pd
import seaborn as sns

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt

ITERATION_PRIORITY = [10000, 1000]
TESTS = [
    "hw-hw-diff",
    "hw-hw-same",
    "hw-hw-same-optimized",
    "hw-sw",
    "hw-sw-optimized",
    "sw-hw",
    "sw-hw-optimized",
    "sw-sw-diff",
    "sw-sw-diff-optimized",
    "sw-sw-same",
    "sw-sw-same-optimized"
]
# TESTS = ["hw-hw-diff"]
TRANSPORT = ["tcp", "udp"]
PAYLOAD_MIN = 0
PAYLOAD_MAX = 10
THROUGHPUT_TIMES = 2
IMAGE_TYPES = ["png", "pdf"]

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

def get_data(path, transport, test_type, iteration, test):

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
                # if test_id != "null":
                #     if test_type == "throughput":
                #         print(data[test_id])
                #         data[test_id] = [sum(data[test_id])/len(data[test_id])]
                #     data[test_id].extend(test_to_cols(test_id))
                #     data[test_id].append(transport)
                test_id = line.split(":")[1].strip()
                if test_id not in data:
                    data[test_id] = []
            elif line.startswith("timing: "):
                cycles = int(line.split(":")[1].strip())                  
                # convert cycles (156.25 MHz, 6.4 ns cycles) to ms
                time_ms = cycles * 6.4 / 1E6
                data[test_id].append(time_ms)
            elif line.startswith("FAILURE"):
                return None
        # if test_type == "throughput":
        #     data[test_id] = [sum(data[test_id])/len(data[test_id])]
        # data[test_id].extend(test_to_cols(test_id))
        # data[test_id].append(transport)

    data_avg = {}
    for test_id, _data in data.items():
        if test_type == "throughput" and len(_data) > 2:
            # print(_data)
            time_sums = [0] * THROUGHPUT_TIMES
            average = [0] * THROUGHPUT_TIMES
            for i in range(0, len(_data), THROUGHPUT_TIMES):
                for j in range(0, THROUGHPUT_TIMES):
                    # print(_data)
                    time_sums[j] += _data[i+j]
            for index, time_sum in enumerate(time_sums):
                # print(time_sum)
                average[index] = time_sum / (len(_data) / 2)
            # data[test_id] = average
            # print(data[test_id])
        else:
            average = _data
        average.extend(test_to_cols(test_id))
        average.append(transport)
        data_avg[test_id] = average
        # data[test_id].extend(test_to_cols(test_id))
        # data[test_id].append(transport)
    
    df = pd.DataFrame.from_dict(data_avg, orient='index').sort_index()
    # if test_type == "throughput":
    #     print(df)
    last_col = len(data_avg[test_id]) - 4
    df = df.rename(columns={last_col: "Test", (last_col+1): "Test_Type", (last_col+2): "Payload", (last_col+3): "Transport"})
    # if test_type == "throughput":
    #     print(df)
    df["iterations"] = iteration
    df["configuration"] = test

    cols = list(df)
    cols.insert(0, cols.pop(cols.index("iterations")))
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

def analyze_data(df):
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

    df["Label"] = df.apply(add_label, axis=1)
    df["Label_short"] = df.apply(shorten_label, axis=1)

    test_type = iterations = df["Test_Type"].values[0]
    if test_type == "Latency":
        iterations = int(df["iterations"].values[0])
        # print(df)
        col = df.loc[:, str(0):str(iterations-1)]
        df["mean"] = col.mean(axis=1)
        df["5th %tile"] = col.apply(np.percentile, args=[5], axis=1)
        df["95th %tile"] = col.apply(np.percentile, args=[95], axis=1)
        df["median"] = col.apply(np.percentile, args=[50], axis=1)

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
        df_new = df[[
            "configuration",
            "Test",
            "Test_Type",
            "Payload",
            "Transport",
            "iterations",
            "Label",
            "Label_short",
            "mean",
            "5th %tile",
            "95th %tile",
            "median",
            "payload_throughput"
        ]].copy()
    else:
        def get_latency(row):
            test_id = row["Test"]
            payload = row["Payload"]

            # test time is in ms
            # test_time = row["1"]
            return row["1"]/row["iterations"]
        
        if "1" in df:
            df["payload_latency"] = df.apply(get_latency, axis=1)
        else:
            df["payload_latency"] = "N/A"

        def get_throughput(row, label):
            test_id = row["Test"]
            payload = row["Payload"]

            # time is in ms
            test_time = row[label]
            if test_time == 0:
                return 0
            if test_id == "Short":
                return 0
            elif test_id == "Strided":
                return STRIDED_METADATA[row["Payload"]][0]*row["iterations"]/test_time*1000*8/1E9
            elif test_id == "Vector":
                return VECTOR_METADATA[row["Payload"]][0]*row["iterations"]/test_time*1000*8/1E9
            else:
                return payload*row["iterations"]/test_time*1000*8/1E9

        df["payload_throughput_0"] = df.apply(get_throughput, args=("0",), axis=1)
        if "1" in df:
            df["payload_throughput_1"] = df.apply(get_throughput, args=("1",), axis=1)
        else:
            df["payload_throughput_1"] = "N/A"

        df_new = df[[
            "configuration",
            "Test",
            "Test_Type",
            "Payload",
            "Transport",
            "iterations",
            "Label",
            "Label_short",
            "payload_latency",
            "payload_throughput_0",
            "payload_throughput_1",
        ]].copy()
        # print(df_new)

    df_new["old_index"] = df_new.index

    return df, df_new


def analyze_latency_iterations(df, path, row_label):
    iterations = int(df["iterations"].values[0])
    try:
        row = df.loc[row_label, str(0):str(iterations-1)]
        row = row.to_frame("latency")
        row.latency = row.latency.astype(float)
        row = row.assign(number=pd.Series(range(0,iterations)).values)
    except KeyError:
        return
    fig, ax = plt.subplots()
    sns.lineplot(x="number", y="latency", data=row, ax=ax)
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
    
    # ax.set_title(df.loc[row_label, "Label"] + " " + df.loc[row_label, "Test_Type"])
    ax.legend()

    fig.tight_layout()
    for image_type in IMAGE_TYPES:
        plt.savefig(os.path.join(path, f"{row_label}.{image_type}"))
    plt.close()

def plot_against_payloads(df, y_axis, y_label, title, filepath, include_short=True):
    if include_short:
        labels = [0]
        labels.extend([2**x * 8 for x in range(PAYLOAD_MIN, PAYLOAD_MAX)])
        hue_order = ["Medium FIFO", "Medium", "Long FIFO", "Long", "Short"]
    else:
        labels = [2**x * 8 for x in range(PAYLOAD_MIN, PAYLOAD_MAX)]
        hue_order = ["Medium FIFO", "Medium", "Long FIFO", "Long"]

    x = np.arange(len(labels))
    width = 0.20

    fig, ax = plt.subplots()
    df_sorted = df.sort_values(["Payload", "Test"])
    if include_short:
        df_subset = df_sorted[
            (df_sorted["Test"] == "Short") | 
            (df_sorted["Test"] == "Medium FIFO") | 
            (df_sorted["Test"] == "Medium") |
            (df_sorted["Test"] == "Long FIFO") |
            (df_sorted["Test"] == "Long")
        ]
    else:
        df_subset = df_sorted[
            (df_sorted["Test"] == "Medium FIFO") | 
            (df_sorted["Test"] == "Medium") |
            (df_sorted["Test"] == "Long FIFO") |
            (df_sorted["Test"] == "Long")
        ]
    sns.barplot(x="Payload", y=y_axis, hue='Test', data=df_subset, ax=ax, hue_order=hue_order)
    ax.set_ylabel(y_label)
    ax.set_xlabel("Payload Size (bytes)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()
    # plt.xticks(rotation=90)
    # ax.set_title(title)

    fig.tight_layout()
    for image_type in IMAGE_TYPES:
        plt.savefig(filepath + "." + image_type)
    plt.close()

def analyze_latency_medians(df, path):
    plot_against_payloads(df, "median", "Time (ms)", "Median Latency by Message Types", os.path.join(path, f"medians_all"))

    df_sorted = df.sort_values(["Payload", "Test"])
    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset["median"], ".")
        ax.set_ylabel("Time (ms)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        # ax.set_title("Median Latency Time of %s Messages" % test)

        fig.tight_layout()
        for image_type in IMAGE_TYPES:
            plt.savefig(os.path.join(path, f"medians_{test}.{image_type}"))
        plt.close()

def analyze_latency_throughput(df, path):
    plot_against_payloads(df, "payload_throughput", "Throughput (Gb/s)", "Payload Throughput (latency mode) by Message Types", os.path.join(path, f"payload_throughput_all"), False)

    df_sorted = df.sort_values(["Payload", "Test"])
    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset["payload_throughput"], ".")
        ax.set_ylabel("Throughput (Gb/s)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        # ax.set_title("Payload Throughput (latency mode) of %s Messages" % test)

        fig.tight_layout()
        for image_type in IMAGE_TYPES:
            plt.savefig(os.path.join(path, f"payload_throughput_{test}.{image_type}"))
        plt.close()


def analyze_one(df, path, test_type):
    if test_type == "latency":
        analyze_latency(df, path)
    else:
        analyze_throughput(df, path)

def analyze_latency(df, path):   
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
    plot_against_payloads(df, payload_label, "Throughput (Gb/s)", "Payload Throughput by Message Types", os.path.join(path, f"{payload_label}_all"), False)

    df_sorted = df.sort_values(["Payload", "Test"])
    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset[payload_label], ".")
        ax.set_ylabel("Throughput (Gb/s)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        # ax.set_title("Payload Throughput of %s Messages" % test)

        fig.tight_layout()
        for image_type in IMAGE_TYPES:
            plt.savefig(os.path.join(path, f"{payload_label}_{test}.{image_type}"))
        plt.close()

def analyze_throughput_latency(df, path):
    plot_against_payloads(df, "payload_latency", "Time (ms)", "Average Latency (throughput mode) by Message Types", os.path.join(path, f"average_all"))

    df_sorted = df.sort_values(["Payload", "Test"])
    for test in ["Medium", "Medium FIFO", "Long", "Long FIFO"]:
        fig, ax = plt.subplots()
        df_subset = df_sorted[df_sorted["Test"] == test]
        ax.plot(df_subset["Payload"], df_subset["payload_latency"], ".")
        ax.set_ylabel("Time (ms)")
        ax.set_xlabel("Payload (bytes)")
        ax.set_xscale("log", basex=2)
        # ax.set_title("Average Latency (throughput mode) Time of %s Messages" % test)

        fig.tight_layout()
        for image_type in IMAGE_TYPES:
            plt.savefig(os.path.join(path, f"averages_{test}.{image_type}"))
        plt.close()

def analyze_throughput(df, path):
    # print(df)

    figure_dir = os.path.join(path, "efficiency")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    for throughput_instance in range(THROUGHPUT_TIMES):
        label = "payload_throughput_" + str(throughput_instance)
        if label in df:
            analyze_throughput_efficiency(df, figure_dir, label)

    if "1" in df:
        figure_dir = os.path.join(path, "latency")
        if not os.path.exists(figure_dir):
            os.makedirs(figure_dir)
        analyze_throughput_latency(df, figure_dir)

def summarize(data_dir, args):
    if not args.load_data:
        data = {}
        data_stats = None
        for test in TESTS:
            print(test)
            data[test] = {}
            for transport in TRANSPORT:
                data[test][transport] = {}
                for test_type in ["latency", "throughput"]:
                    data[test][transport][test_type] = {}
                    for iteration in ITERATION_PRIORITY:
                        test_data = "ubench_" + test + "_" + test_type + "_" + transport + "_" + str(iteration) + ".txt"
                        test_data_path = os.path.join(data_dir, test_data)
                        if os.path.exists(test_data_path):
                            df = get_data(test_data_path, transport, test_type, iteration, test)
                            if df is None:
                                continue
                            df, df_stats = analyze_data(df)
                            # print(df)
                            data[test][transport][test_type][str(iteration)] = df
                            if data_stats is None:
                                data_stats = df_stats
                            else:
                                data_stats = pd.concat([data_stats, df_stats], ignore_index=True)
    else:
        with open("data.pickle", "rb") as f:
            data = pickle.load(f)
        with open("data_stats.pickle", "rb") as f:
            data_stats = pickle.load(f)

    # print(data_stats)

    if args.analyze_single_data:
        for test in TESTS:
            for transport in TRANSPORT:
                for test_type in ["latency", "throughput"]:
                    for iteration in ITERATION_PRIORITY:
                        try:
                            df = data[test][transport][test_type][str(iteration)]
                        except KeyError as e:
                            continue
                        else:
                            figure_dir = os.path.join(data_dir, "build", test, transport, test_type, str(iteration))
                            if not os.path.exists(figure_dir):
                                os.makedirs(figure_dir)
                            analyze_one(df, figure_dir, test_type)

    if args.analyze_group_data:
        print(data_stats)
    
    if args.save_data:
        with open("data.pickle", "wb") as f:
            pickle.dump(data, f)
        with open("data_stats.pickle", "wb") as f:
            pickle.dump(data_stats, f)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract timing from the microbenchmark test")
    parser.add_argument("--dir", type=str, default="./data", help="Path to directory with data")
    parser.add_argument("--save-data", dest="save_data", action="store_true", help="Save the parsed data as a pickle for reuse")
    parser.add_argument("--load-data", dest="load_data", action="store_true", help="Load the pickled data instead of reparsing")
    parser.add_argument("--analyze-single-data", dest="analyze_single_data", action="store_true", help="Load the pickled data instead of reparsing")
    parser.add_argument("--analyze-group-data", dest="analyze_group_data", action="store_true", help="Load the pickled data instead of reparsing")
    parser.set_defaults(
        save_data=False,
        load_data=True,
        analyze_single_data=True,
        analyze_group_data=False
    )

    args = parser.parse_args()
    full_path = os.path.abspath(args.dir)
    print(args)
    if os.path.exists(full_path):
        summarize(full_path, args)
    else:
        print("%s does not exist" % args.dir)

