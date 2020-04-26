import argparse
import os
# import statistics
import numpy as np
import pandas as pd

import matplotlib
# Force matplotlib to not use any Xwindows backend.
matplotlib.use('Agg')
import matplotlib.pyplot as plt

ITERATION_COUNT = 1000
TESTS = ["sw-sw-same"]
TRANSPORT = ["tcp"]
PAYLOAD = 10

N = ITERATION_COUNT-1

def get_data(path, transport):

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
                data[test_id] = []
            elif line.startswith("timing: "):
                cycles = int(line.split(":")[1].strip())
                # convert cycles (156.25 MHz, 6.4 ns cycles) to ms
                time_ms = cycles * 6.4 / 1E6
                data[test_id].append(time_ms)
        data[test_id].extend(test_to_cols(test_id))
        data[test_id].append(transport)
    df = pd.DataFrame.from_dict(data, orient='index').sort_index()
    df = df.rename(columns={ITERATION_COUNT: "Test", (ITERATION_COUNT+1): "Test_Type", (ITERATION_COUNT+2): "Payload", (ITERATION_COUNT+3): "Transport"})
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
        return 2**int(row["Payload"]) * 8
    df["Payload"] = df.apply(reset_short_payload, axis=1)

    return df


def analyze_iterations(df, path, row_label):
    try:
        row = df.loc[row_label, str(0):str(N)]
    except KeyError:
        return
    fig, ax = plt.subplots()
    ax.plot(pd.Series(range(0,ITERATION_COUNT)), row, label="_hidden")
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


def analyze_medians(df, path):
    fig, ax = plt.subplots()
    df_sorted = df.sort_values(["Payload", "Test"])
    ax.bar(df_sorted["Label_short"], df_sorted["median"])
    ax.set_ylabel("Time (ms)")
    ax.set_xlabel("Test")
    plt.xticks(rotation=90)
    ax.set_title("Median Latency Time by Message Types")

    fig.tight_layout()
    plt.savefig(os.path.join(path, "medians_all.png"))
    plt.close()

    for test in ["Medium"]:
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


def analyze_one(df, path):
    def add_label(row):
        test_id = row["Test"]
        payload = row["Payload"]
        if test_id != "Short":
            title = "%s Message (%dB payload)" % (test_id, payload)
        else:
            title = "%s Message" % (test_id)
        return title

    def shorten_label(row):
        test_id = row["Test"]
        if test_id == "Short":
            test_initial = "S"
        elif test_id.startswith("Medium"):
            test_initial = "M"
        else:
            test_initial = "L"
        
        if "FIFO" in test_id:
            test_initial += " F"
        payload =row["Payload"]
        if test_id != "Short":
            test_payload = " (%dB)" % payload
        else:
            test_payload = ""
        return test_initial + test_payload

    col = df.loc[:, str(0):str(N)]
    df["mean"] = col.mean(axis=1)
    df["5th %tile"] = col.apply(np.percentile, args=[5], axis=1)
    df["95th %tile"] = col.apply(np.percentile, args=[95], axis=1)
    df["median"] = col.apply(np.percentile, args=[50], axis=1)
    df["Label"] = df.apply(add_label, axis=1)
    df["Label_short"] = df.apply(shorten_label, axis=1)

    print(df)
    
    df_rows = []
    df_rows.append({"label": "short_latency_0", "title": "Short Message Latency"})
    for i in range(PAYLOAD):
        payload = 2**i * 8
        df_rows.append({"label": "medium_latency_" + str(i), "title": "Medium Latency (%d byte payload)" % payload})
        df_rows.append({"label": "medium-fifo_latency_" + str(i), "title": "Medium FIFO Latency (%d byte payload)" % payload})
        df_rows.append({"label": "long_latency_" + str(i), "title": "Long Latency (%d byte payload)" % payload})
        df_rows.append({"label": "long-fifo_latency_" + str(i), "title": "Long FIFO Latency (%d byte payload)" % payload})

    # figure_dir = os.path.join(path, "iterations")
    # if not os.path.exists(figure_dir):
    #     os.makedirs(figure_dir)
    # for row in df_rows:
    #     analyze_iterations(df, figure_dir, row["label"])
    
    figure_dir = os.path.join(path, "medians")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    analyze_medians(df, figure_dir)

def summarize(data_dir):
    data = {}
    for test in TESTS:
        data[test] = {}
        for transport in TRANSPORT:
            test_data = "microbenchmarks_" + test + "_" + transport + "_raw.txt"
            test_data_path = os.path.join(data_dir, test_data)
            if os.path.exists(test_data_path):
                df = get_data(test_data_path, transport)
                data[test][transport] = df
                figure_dir = os.path.join(data_dir, "build", test, transport)
                if not os.path.exists(figure_dir):
                    os.makedirs(figure_dir)
                analyze_one(df, figure_dir)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract timing from the microbenchmark test")
    parser.add_argument("dir", type=str, help="Path to directory with data")

    args = parser.parse_args()
    full_path = os.path.abspath(args.dir)
    if os.path.exists(full_path):
        summarize(full_path)
    else:
        print("%s does not exist" % args.dir)

