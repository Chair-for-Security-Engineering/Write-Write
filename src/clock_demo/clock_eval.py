import statistics
import numpy as np
import matplotlib.pyplot as plt

def get_period(xval):
    period = []
    for clk in range(2, len(xval), 2):
        period.append(xval[clk]-xval[clk-2])
    print(f"Mean Period: {sum(period)/len(period)}")
    #print(statistics.stdev(period))
    #print(np.percentile(period, 10))

#get cycle-to-cycle jitter
def get_cc_jitter(xval):
    max = 0
    tick = 0
    for clk in range(2, len(xval)-2):
        if abs((xval[clk]-xval[clk-2])-(xval[clk+2]-xval[clk])) > max:
            max = abs((xval[clk]-xval[clk-2])-(xval[clk+2]-xval[clk]))
            tick = clk
    print(f"CC Jitter: {max} - @{tick}")

def get_avg_jitter(xval):
    sum = 0
    cnt = 0
    for clk in range(2, len(xval)-2):
        cnt += 1 
        sum += abs((xval[clk]-xval[clk-2])-(xval[clk+2]-xval[clk]))
    print(f"AVG Jitter: {sum/cnt}")

def get_sync_error(x1, x2):
    sum = 0
    cnt = 0
    max = 0
    for clk in range(2, len(x1)):
        cnt += 1 
        #print((xval[clk]-xval[clk-2])-(xval[clk+2]-xval[clk]))
        val = abs((x1[clk])-(x2[clk]))
        sum += val
        if val > max:
            max = val
    print(f"Sync error {max}, Avg: {sum/cnt}")

# Convert High and Low signal to rectangular shape
def extrapolate(data):
    res = {}
    for key, trace in data.items():
        res[key] = {"x": [], "y": []}
        for i in range(len(trace["x"])):
            res[key]["x"].append(data[key]["x"][i]-1)
            res[key]["x"].append(data[key]["x"][i])
            res[key]["y"].append(1-data[key]["y"][i])
            res[key]["y"].append(data[key]["y"][i])
    return res


def plot(data):
    for _, trace in data.items():
        plt.plot(trace["x"], trace["y"], alpha=0.7)
    plt.savefig("sync.png")

def read_data(files):
    data = {}
    for fname in files:
        f = open(fname, "r")
        data[fname] = {"x": [], "y": []}
        for line in f:
            row = line.split()
            data[fname]["x"].append(int(row[0]))
            data[fname]["y"].append(int(row[1]))
    return data

files = ["a.txt", "b.txt"]
data = read_data(files)
plot(extrapolate(data))
get_sync_error(data[files[0]]["x"], data[files[1]]["x"])
get_cc_jitter(data[files[0]]["x"][10:])
get_avg_jitter(data[files[0]]["x"][10:])
get_period(data[files[0]]["x"][10:])
