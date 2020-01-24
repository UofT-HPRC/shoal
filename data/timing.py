import argparse
import os
import subprocess

def parse_results(results):
    iterations = results.split("Timing Finished\n")
    timing = {}
    for iteration in iterations:
        lines = iteration.splitlines()
        for line in lines:
            if line.startswith("Timing:"):
                time_measure = line.split(":")
                if time_measure[1] not in timing:
                    timing[time_measure[1]] = 0
                timing[time_measure[1]] += float(time_measure[2])
    return timing

def benchmark(app_path, run_count):
    result = ""
    for i in range(run_count):
        result += subprocess.check_output([app_path], shell=True)
    timings = parse_results(result)
    for key, value in sorted(timings.iteritems()):
        print("%s: %.15g" % (key, value/run_count))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract timing from C++ application")
    parser.add_argument("app", type=str, help="Path to executable")
    parser.add_argument("-t", "--times", type=int, help="Number of times to run", default=100)

    args = parser.parse_args()
    full_path = os.path.abspath(args.app)
    if os.path.exists(full_path):
        benchmark(full_path, args.times)
    else:
        print("%s does not exist" % args.app)