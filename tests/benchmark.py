import argparse
import os
# import statistics
import numpy as np

def get_data(path):
    data = {}
    test_id = "null"
    with open(path, "r") as f:
        for line in f:
            if line.startswith("test: "):
                test_id = line.split(":")[1].strip()
                data[test_id] = []
            elif line.startswith("timing: "):
                cycles = int(line.split(":")[1].strip())
                data[test_id].append(cycles)

    return data

def print_data(label, value):
    # convert cycles (156.25 MHz, 6.4 ns cycles) to ms
    print("  " + label + ": " + "{:.3f}".format(value * 6.4 / 1E6) + "ms")

def analyze_one(data):
    print_data("average", np.mean(data))
    print_data("1st %tile", np.percentile(data, 1))
    print_data("5th %tile", np.percentile(data, 5))
    print_data("median", np.median(data))
    print_data("95th %tile", np.percentile(data, 95))
    print_data("99th %tile", np.percentile(data, 99))


def analyze(data):
    for key, value in data.items():
        print(key)
        analyze_one(np.array(value))

def summarize(path):
    data = get_data(path)
    analyze(data)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract timing from the microbenchmark test")
    parser.add_argument("data", type=str, help="Path to data")

    args = parser.parse_args()
    full_path = os.path.abspath(args.data)
    if os.path.exists(full_path):
        summarize(full_path)
    else:
        print("%s does not exist" % args.data)

