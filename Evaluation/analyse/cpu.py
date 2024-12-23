import time

import numpy as np
import pandas
from matplotlib import pyplot as plt


def get_cpu_usage(cpu_files, gc_files):
    result = []
    for cpu_file, gc_file in zip(cpu_files, gc_files):
        gc_time_data = pandas.read_csv(gc_file)
        gc_times = []
        max_time = 0
        for index, row in gc_time_data.iterrows():
            start_time = phrase_time(row['start time'])
            end_time = phrase_time(row['end time'])
            if end_time > max_time:
                max_time = end_time
            gc_times.append((start_time, end_time))
        min_time = gc_times[0][0]

        cpu_usage_data = pandas.read_csv(cpu_file)
        cpu_util = 0
        time_point = 0
        for index, row in cpu_usage_data.iterrows():
            timestamp = phrase_time(row['time'])
            cur_time_point = 0
            if min_time <= timestamp <= max_time:
                # for gc_time in gc_times:
                #     if gc_time[0] <= timestamp <= gc_time[1]:
                #         cur_time_point += 1
                # if cur_time_point >= 0:
                #     # cpu_util += (row['cpu%']) / (cur_time_point + 1.35)
                thread = row['thread']
                if thread == 0:
                    continue
                cpu_util += (row['cpu%']) / thread
                time_point += 1

        result.append(cpu_util / time_point)

    return result


def phrase_time(time_str):
    # dt = datetime.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    if isinstance(time_str, str):
        # print(time_str,len(time_str))
        time_array = time.strptime(time_str, '%Y-%m-%d %H:%M:%S')
        timestamp = int(time.mktime(time_array))
        return timestamp
    else:
        return time_str // 1000000


def main():
    params = ["0.6", "0.5", "0.4", "0.3"]
    paths = ["/Users/fenghao/Desktop/ASPLOS2024 GCoffloading/motivation_test/ssd_ycsb/大数据量/" + param + "-200M/" for
             param in params]
    # paths = ["/Users/fenghao/Desktop/offload/overall/titan-uniform/"]

    cpu_files = [path + "CPU" for path in paths]
    gc_files = [path + "GC_TIME" for path in paths]

    io_list = [(87, 24), (119, 45), (145, 70), (191, 113)]
    colors = [(144 / 255, 201 / 255, 231 / 255), (219 / 255, 49 / 255, 36 / 255), (252 / 255, 140 / 255, 90 / 255)]
    output_list = [io[1] for io in io_list]
    input_list = [io[0] for io in io_list]

    # cpu_usage = get_cpu_usage(cpu_files, gc_files)
    # print(cpu_usage)
    cpu_usage = [i / 2.3 for i in [221, 203, 195, 182]]

    plt.rcParams['font.sans-serif'] = ['Times New Roman']
    plt.rcParams['axes.unicode_minus'] = False
    fig = plt.figure(figsize=(4, 3))
    ax = fig.subplots()

    x_data = ['60', '50', '40', '30']
    x = np.arange(len(x_data))
    width = 0.30
    bottom = input_list

    ax.bar(x - width / 2, cpu_usage, width, label="CPU Usage", color=colors[0])
    ax.set_ylabel("Normalized Utilization (%)", fontsize=12)
    ax.set_ylim(ymin=50, ymax=100)
    ax.set_xlabel("Garbage Ratio (%)", fontsize=12)

    ax2 = ax.twinx()
    ax2.bar(x + width / 2, input_list, width, label="Read", color=colors[1])
    ax2.bar(x + width / 2, output_list, width, bottom=bottom, label='Write', color=colors[2])
    ax2.set_ylabel("Data Size (GB)", fontsize=12, rotation=-90, labelpad=13)
    ax2.set_ylim(ymin=0, ymax=350)

    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2, markerscale=7, ncol=3,
               bbox_to_anchor=(1.15, 1.2), loc=0, frameon=False, fontsize=11)

    ax.set_xticks(x)
    ax.set_xticklabels(x_data)

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/motivation-throughput.jpg', dpi=500)


if __name__ == '__main__':
    # main()
    params = ["Titan","AegonKV"]
    paths = ["/Users/fenghao/Desktop/S2/sensitive/batch-32/" + param for param in params]
    # params = ["RocksDB", "RocksDB-Blob", "Titan", "Titan-no", "DiffKV", "AegonKV"]
    # params = ["AegonKV","Titan", "DiffKV", "RocksDB-Blob", "RocksDB"]
    # paths = ["/Users/fenghao/Desktop/S2/sensitive/db-40G/" + param for param in params]
    # paths = ["/Users/fenghao/Desktop/S2/" + param + "/thread-8" for param in params]
    cpu_files = [path + "/CPU" for path in paths]
    gc_files = [path + "/GC_TIME" for path in paths]

    result = []
    for cpu_file, gc_file in zip(cpu_files, gc_files):
        cpu_usage_data = pandas.read_csv(cpu_file)
        cpu_util = 0
        time_point = 0
        i = 0

        gc_times = []
        if 'AegonKV' in gc_file:
            gc_time_data = pandas.read_csv(gc_file)
            max_time = 0
            for index, row in gc_time_data.iterrows():
                start_time = phrase_time(row['start time'])
                end_time = phrase_time(row['end time'])
                if end_time > max_time:
                    max_time = end_time
                gc_times.append((start_time, end_time))
            min_time = gc_times[0][0]

        for index, row in cpu_usage_data.iterrows():
            i += 1
            if i > 4000:
                break
            thread = row['thread']
            if thread == 0:
                continue

            if 'AegonKV' in gc_file:
                timestamp = phrase_time(row['time'])
                if min_time <= timestamp <= max_time:
                    for gc_time in gc_times:
                        if gc_time[0] <= timestamp <= gc_time[1]:
                            thread += 1

            cpu_util += min((row['cpu%']) / thread, 100)

            # cpu_util += (row['cpu%'])
            time_point += 1
        if time_point==0:
            result.append(0)
        else:
            result.append(cpu_util / time_point)
    print(result)
