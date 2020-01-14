import argparse
import os
import subprocess
import xml.etree.ElementTree as ET

import pandas as pd

# https://stackoverflow.com/a/40170206
def git_version():
    def _minimal_ext_cmd(cmd):
        # construct minimal environment
        env = {}
        for k in ['SYSTEMROOT', 'PATH']:
            v = os.environ.get(k)
            if v is not None:
                env[k] = v
        # LANGUAGE is used on win32
        env['LANGUAGE'] = 'C'
        env['LANG'] = 'C'
        env['LC_ALL'] = 'C'
        out = subprocess.Popen(cmd, stdout = subprocess.PIPE, env=env).communicate()[0]
        return out

    try:
        out = _minimal_ext_cmd(['git', 'rev-parse', 'HEAD'])
        GIT_REVISION = out.strip().decode('ascii')
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


def main(path):
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
                df.append(pd.Series(name=row_index))
                df.at[row_index, "Project"] = project
                df.at[row_index, "Solution"] = solution
                df.at[row_index, "Report"] = report

                parse_report(df, row_index, report_path)

                row_index += 1

    df.to_csv(os.path.join(os.getcwd(), git_version() + ".txt"), index=False)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Extract timing from Vivado HLS projects")
    parser.add_argument("path", type=str, help="Full path to vivado hls projects")
    # parser.add_argument("-s", "--solutions", nargs="+", help="Only extract these solutions")

    args = parser.parse_args()
    main(args.path)
