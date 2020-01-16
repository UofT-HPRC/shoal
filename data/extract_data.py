import argparse
import os
import subprocess
import xml.etree.ElementTree as ET

import numpy as np
import pandas as pd

# https://stackoverflow.com/a/40170206
def git_version():
    def _minimal_ext_cmd(cmd):
        # construct minimal environment
        env = {}
        for k in ["SYSTEMROOT", "PATH"]:
            v = os.environ.get(k)
            if v is not None:
                env[k] = v
        # LANGUAGE is used on win32
        env["LANGUAGE"] = "C"
        env["LANG"] = "C"
        env["LC_ALL"] = "C"
        out = subprocess.Popen(cmd, stdout = subprocess.PIPE, env=env).communicate()[0]
        return out

    try:
        out = _minimal_ext_cmd(["git", "rev-parse", "HEAD"])
        GIT_REVISION = out.strip().decode("ascii")
    except OSError:
        GIT_REVISION = "Unknown"

    return GIT_REVISION


def get_child_directories(path):
    """
    Given a path to a Vivado HLS project, determine the solutions that exist here
    
    Args:
        path (str): Path to the Vivado HLS project
    """

    # in HLS 2017.2, solutions are subdirectories of the root directory
    all_directories = next(os.walk(path))[1]

    # remove hidden directories (part of HLS)
    directories = [directory for directory in all_directories if not directory.startswith(".")]
    
    # add path for full path
    full_directories = [os.path.join(path, solution) for solution in directories]
    return directories, full_directories

def get_solution_reports(solution):
    report_path = os.path.join(solution, "syn", "report")
    if os.path.exists(report_path):
        files = next(os.walk(report_path))[2]
        reports = [report for report in files if report.endswith(".xml")]
        full_reports = [os.path.join(report_path, report) for report in reports]
    else:
        reports = []
        full_reports = []
    return reports, full_reports


def parse_report(df, row, report):
    root = ET.parse(report).getroot()

    def ProductFamily():
        df.at[row, "ProductFamily"] = root.find("UserAssignments/ProductFamily").text

    def Part():
        df.at[row, "Part"] = root.find("UserAssignments/Part").text

    def TargetClockPeriod():
        period = root.find("UserAssignments/TargetClockPeriod").text
        unit = root.find("UserAssignments/unit").text
        df.at[row, "TargetClockPeriod"] = period + unit

    def ClockUncertainty():
        period = root.find("UserAssignments/ClockUncertainty").text
        unit = root.find("UserAssignments/unit").text
        df.at[row, "ClockUncertainty"] = period + unit

    def EstimatedClockPeriod():
        period = root.find("PerformanceEstimates//SummaryOfTimingAnalysis/EstimatedClockPeriod").text
        unit = root.find("PerformanceEstimates/SummaryOfTimingAnalysis/unit").text
        df.at[row, "EstimatedClockPeriod"] = period + unit

    def Best_caseLatency():
        df.at[row, "Best-caseLatency"] = root.find("PerformanceEstimates/SummaryOfOverallLatency/Best-caseLatency").text

    def Average_caseLatency():
        df.at[row, "Average-caseLatency"] = root.find("PerformanceEstimates/SummaryOfOverallLatency/Average-caseLatency").text

    def Worst_caseLatency():
        df.at[row, "Worst-caseLatency"] = root.find("PerformanceEstimates/SummaryOfOverallLatency/Worst-caseLatency").text

    def Interval_min():
        df.at[row, "Interval-min"] = root.find("PerformanceEstimates/SummaryOfOverallLatency/Interval-min").text

    def Interval_max():
        df.at[row, "Interval-max"] = root.find("PerformanceEstimates/SummaryOfOverallLatency/Interval-max").text

    def BRAM_18K():
        df.at[row, "BRAM_18K"] = root.find("AreaEstimates/Resources/BRAM_18K").text

    def FF():
        df.at[row, "FF"] = root.find("AreaEstimates/Resources/FF").text

    def LUT():
        df.at[row, "LUT"] = root.find("AreaEstimates/Resources/LUT").text

    def DSP48E():
        df.at[row, "DSP48E"] = root.find("AreaEstimates/Resources/DSP48E").text

    def Available_BRAM_18K():
        df.at[row, "Available_BRAM_18K"] = root.find("AreaEstimates/AvailableResources/BRAM_18K").text

    def Available_FF():
        df.at[row, "Available_FF"] = root.find("AreaEstimates/AvailableResources/FF").text

    def Available_LUT():
        df.at[row, "Available_LUT"] = root.find("AreaEstimates/AvailableResources/LUT").text

    def Available_DSP48E():
        df.at[row, "Available_DSP48E"] = root.find("AreaEstimates/AvailableResources/DSP48E").text


    ProductFamily()
    Part()
    TargetClockPeriod()
    ClockUncertainty()
    EstimatedClockPeriod()
    Best_caseLatency()
    Average_caseLatency()
    Worst_caseLatency()
    Interval_min()
    Interval_max()
    BRAM_18K()
    FF()
    LUT()
    DSP48E()
    Available_BRAM_18K()
    Available_FF()
    Available_LUT()
    Available_DSP48E()


def extract(path):
    df = pd.DataFrame(columns=[
        "Project",
        "Solution",
        "Report",
        "ProductFamily",
        "Part",
        "TargetClockPeriod",
        "ClockUncertainty",
        "EstimatedClockPeriod",
        "Best-caseLatency",
        "Average-caseLatency",
        "Worst-caseLatency",
        "Interval-min",
        "Interval-max",
        "BRAM_18K",
        "FF",
        "LUT",
        "DSP48E",
        "Available_BRAM_18K",
        "Available_FF",
        "Available_LUT",
        "Available_DSP48E",
    ])
    row_index = 0

    projects, project_paths = get_child_directories(path)
    for project, project_path in list(zip(projects, project_paths)):
        solutions, solution_paths = get_child_directories(project_path)
        for solution, solution_path in list(zip(solutions, solution_paths)):
            reports, report_paths = get_solution_reports(solution_path)
            for report, report_path in list(zip(reports, report_paths)):
                # df.append(pd.Series(name=row_index))
                df.at[row_index, "Project"] = project
                df.at[row_index, "Solution"] = solution
                df.at[row_index, "Report"] = report

                parse_report(df, row_index, report_path)

                row_index += 1

    df.to_csv(os.path.join(os.getcwd(), git_version() + ".txt"), index=False)

# https://stackoverflow.com/a/38421614
def diff_pd(df1, df2):
    """Identify differences between two pandas DataFrames"""
    assert (df1.columns == df2.columns).all(), \
        "DataFrame column names are different"
    if any(df1.dtypes != df2.dtypes):
        "Data Types are different, trying to convert"
        df2 = df2.astype(df1.dtypes)
    if df1.equals(df2):
        return None
    else:
        # need to account for np.nan != np.nan returning True
        diff_mask = (df1 != df2) & ~(df1.isnull() & df2.isnull())
        ne_stacked = diff_mask.stack()
        changed = ne_stacked[ne_stacked]
        changed.index.names = ["id", "col"]
        difference_locations = np.where(diff_mask)
        changed_from = df1.values[difference_locations]
        changed_to = df2.values[difference_locations]
        return pd.DataFrame({"from": changed_from, "to": changed_to},
                            index=changed.index)


def compare(old_file, new_file):
    old_df = pd.read_csv(old_file)
    new_df = pd.read_csv(new_file)

    old_df["index"] = old_df[[
        "Project", "Solution", "Report", "ProductFamily", "Part", "TargetClockPeriod", "ClockUncertainty"
    ]].apply(lambda x: "-".join(x), axis=1)
    old_df.set_index("index", inplace=True)
    new_df["index"] = new_df[[
        "Project", "Solution", "Report", "ProductFamily", "Part", "TargetClockPeriod", "ClockUncertainty"
    ]].apply(lambda x: "-".join(x), axis=1)
    new_df.set_index("index", inplace=True)

    for idx in old_df.index:
        if idx not in new_df.index:
            new_df = new_df.append(pd.Series(name=idx))
            for column in new_df.columns:
                new_df.at[idx, column] = -1

    for idx in new_df.index:
        if idx not in old_df.index:
            old_df = old_df.append(pd.Series(name=idx))
            for column in old_df.columns:
                old_df.at[idx, column] = -1

    old_df.sort_index(inplace=True)
    new_df.sort_index(inplace=True)

    compare_df = diff_pd(old_df, new_df)

    # find all blocks that no longer exist in the new data
    deleted_dict = dict(compare_df.index[compare_df["to"] == -1].tolist())
    deleted_indices = list(deleted_dict.keys())
    compare_df = compare_df.drop(compare_df[compare_df.to == -1].index)
    for idx in deleted_indices:
        multi_idx = (idx, "All")
        compare_df = compare_df.append(pd.Series(name=multi_idx))
        compare_df.at[multi_idx, "from"] = "Existed"
        compare_df.at[multi_idx, "to"] = "Deleted"

    # find all blocks that now exist in the new data
    deleted_dict = dict(compare_df.index[compare_df["from"] == -1].tolist())
    deleted_indices = list(deleted_dict.keys())
    compare_df = compare_df.drop(compare_df[compare_df["from"] == -1].index)
    for idx in deleted_indices:
        multi_idx = (idx, "All")
        compare_df = compare_df.append(pd.Series(name=multi_idx))
        compare_df.at[multi_idx, "from"] = "Does not exist"
        compare_df.at[multi_idx, "to"] = "New"

    compare_df.to_csv(os.path.join(os.getcwd(), os.path.basename(old_file)[0:6] + "_" + os.path.basename(new_file)[0:6] + ".txt"))

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Extract timing from Vivado HLS projects")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("-e", "--extract", type=str, help="Path to vivado hls projects")
    group.add_argument("-c", "--compare", nargs="+", type=str, help="Data to compare")
    # parser.add_argument("-s", "--solutions", nargs="+", help="Only extract these solutions")

    args = parser.parse_args()
    if args.extract is None:
        if len(args.compare) == 2:
            compare(os.path.abspath(args.compare[0]), os.path.abspath(args.compare[1]))
        else:
            raise NotImplementedError
    else:
        extract(os.path.abspath(args.extract))
