import pickle
import os

from benchmark import plot_against_payloads

def libGal_vs_shoal(gal_data, shoal_data, path):
    figure_dir = os.path.join(path, "libGal_vs_shoal")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)
    
    for test in ["same", "diff"]:
        figure_dir_2 = os.path.join(figure_dir, test)
        if not os.path.exists(figure_dir_2):
            os.makedirs(figure_dir_2)
        for test_mode in ["throughput_0", "throughput_1"]:
            if test == "same":
                gal_test = "no_busy"
            else:
                gal_test = "no_busy_diff"
            gal_data_subset = gal_data[
                (gal_data["test"] == gal_test) &
                (gal_data["test_id"] == "reply") &
                (gal_data["test_mode"] == test_mode) &
                (gal_data["wr_mode"] == "packet_malloc-packet_mem") &
                (gal_data["payload"] <= 4096)
            ]
            gal_data_subset = gal_data_subset.append([gal_data_subset]*3, ignore_index=True)
            shoal_data_subset = shoal_data[
                (shoal_data["configuration"] == f"sw-sw-{test}-optimized") &
                (shoal_data["Test_Type"] == "Throughput") &
                ((shoal_data["Test"] == "Medium") |
                (shoal_data["Test"] == "Medium FIFO") |
                (shoal_data["Test"] == "Long") |
                (shoal_data["Test"] == "Long FIFO"))
            ]
            shoal_data_subset = shoal_data_subset.reset_index(drop=True)

            shoal_data_subset["percent_change"] = (shoal_data_subset[f"payload_{test_mode}"] - gal_data_subset["time"]) / gal_data_subset["time"] * 100

            plot_against_payloads(shoal_data_subset, "percent_change", "Percent Difference (%)", "shoal throughput vs libGalapagos", os.path.join(figure_dir_2, f"payload_{test_mode}.png"), False)




if __name__ == "__main__":
    with open("libGalapagos.pickle", "rb") as f:
        gal_data = pickle.load(f)
    with open("data_stats.pickle", "rb") as f:
        shoal_data = pickle.load(f)

    data_dir = os.path.abspath("./data")
    figure_dir = os.path.join(data_dir, "build", "cross")
    if not os.path.exists(figure_dir):
        os.makedirs(figure_dir)

    shoal_data["payload_throughput_0"] = shoal_data["payload_throughput_0"].apply(lambda x: x*1000)
    shoal_data["payload_throughput_1"] = shoal_data["payload_throughput_1"].apply(lambda x: x*1000)

    libGal_vs_shoal(gal_data, shoal_data, figure_dir)