import re
import time

import numpy as np
import pandas
from matplotlib import pyplot as plt


def _init_plt_env():
    plt.rcParams['font.sans-serif'] = ['Times New Roman']
    plt.rcParams['axes.unicode_minus'] = False


def _gc_count_at_time(gc_list, time_list):
    result = []
    for time_point in time_list:
        count = 0
        for gc in gc_list:
            if gc[0] <= time_point < gc[1]:
                count += 1
        result.append(count)
    return result


def _phrase_time(time_str):
    # dt = datetime.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    if isinstance(time_str, str):
        time_array = time.strptime(time_str, '%Y-%m-%d %H:%M:%S')
        timestamp = int(time.mktime(time_array))
        return timestamp
    else:
        return time_str // 1000000


def _get_gc_time(path):
    gc_time_data = pandas.read_csv(path)

    result = []
    for index, row in gc_time_data.iterrows():
        start_time = _phrase_time(row['start time'])
        end_time = _phrase_time(row['end time'])
        duration = end_time - start_time
        result.append((start_time, end_time, duration))

    return result


def _get_gc_info(path, micros=True):
    gc_time_data = pandas.read_csv(path)

    result = []
    for index, row in gc_time_data.iterrows():
        start_time = int(row['start time']) // 1000000 if micros else _phrase_time(row['start time'])
        end_time = int(row['end time']) // 1000000 if micros else _phrase_time(row['end time'])
        duration = end_time - start_time
        result.append((start_time, end_time, duration, int(row['read blob num']), int(row['write back num'])))

    return result


def _get_ops(path, operations):
    re_strs = []
    re_strs.append(("ALL", "[0-9]+ operations"))
    for op in operations:
        re_str = r"\[" + op + ": Count=[0-9]+"
        re_strs.append((op, re_str))

    result_list = []
    with open(path, encoding='utf-8') as file:
        pre_ops_map = {key: 0 for key in operations}
        for line in file:
            res = []
            time_str = line[:19]
            res.append(time_str)

            for re_str in re_strs:
                match_res = re.search(re_str[1], line)
                re_res = match_res.group()
                if re_str[0] == "ALL":
                    res.append(int(re_res[:-(len(" operations"))]))
                else:
                    ops = int(re_res[len("[" + re_str[0] + ": Count="):])
                    res.append(ops - pre_ops_map[re_str[0]])
                    pre_ops_map[re_str[0]] = ops
            result_list.append(res)
    return result_list


def _get_gc_stats_from_file(path):
    gc_time_data = pandas.read_csv(path)

    time_splits = ['read lsm micros', 'update lsm micros', 'read blob micros', 'write blob micros']
    ops_name = ['read lsm num', 'read blob num', 'write back num']

    gc_stats_result = []
    gc_time_sum = [0 for _ in range(len(time_splits) + 1)]
    ops_sum = [0 for _ in range(len(ops_name))]

    for index, row in gc_time_data.iterrows():
        duration = int(row['end time']) - int(row['start time'])
        stats = []
        for i in range(len(time_splits)):
            stats.append(int(row[time_splits[i]]) / duration)
            gc_time_sum[i] += int(row[time_splits[i]])
        gc_time_sum[-1] += duration
        gc_stats_result.append(stats)
        for i in range(len(ops_name)):
            ops_sum[i] += int(row[ops_name[i]])

    return gc_stats_result, gc_time_sum, ops_sum


def _qps_draw(ax, x_data, y_data):
    ax.scatter(x_data, y_data, c="b", s=1, label="Throughput")
    ax.set_xlabel("Elapsed Seconds", fontsize=10)
    ax.set_ylabel("Throughput (ops/sec)", fontsize=10)
    ax.tick_params("y", color="b")


def _gc_draw(ax, x_data, y_data):
    ax.scatter(x_data, y_data, c="r", s=1, label="GC Count")
    ax.set_ylabel("GC Count (ops)", fontsize=10, rotation=-90, labelpad=8)
    ax.set_yticks(np.arange(0, max(y_data) + 3, 3))
    ax.tick_params("y", color="r")


def _double_plot(ax, p1_data):
    _qps_draw(ax, p1_data[0], p1_data[1])
    ax_tx = ax.twinx()
    _gc_draw(ax_tx, p1_data[0], p1_data[2])
    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax_tx.get_legend_handles_labels()
    ax_tx.legend(lines + lines2, labels + labels2, markerscale=7, ncol=2,
                 bbox_to_anchor=(0.85, 1.25), loc=0, frameon=False,fontsize=11)


def _double_bar_plot(ax, x_data, plot_data, plot_label, bar_label, y1_data, y2_data, y1_label, y2_label, plot_label2,
                     colors, bar_lim=None, plot_lim=None):
    x = np.arange(len(x_data))
    width = 0.35

    ax.bar(x - width / 2, y1_data, width, label=y1_label, color=colors[0])
    ax.set_ylabel(bar_label, fontsize=10)
    ax.bar(x + width / 2, y2_data, width, label=y2_label, color=colors[1])
    ax.set_xticks(x)
    ax.set_xticklabels(x_data)
    if bar_lim is not None:
        ax.set_ylim(ymin=bar_lim[0], ymax=bar_lim[1])

    ax2 = ax.twinx()
    ax2.plot(x_data, plot_data,
             color=(246 / 255, 111 / 255, 105 / 255),
             label=plot_label2,
             linestyle='-',
             linewidth=3,
             marker='o',
             markersize=5,
             markeredgecolor='blue',
             markerfacecolor='black')
    ax2.set_ylabel(plot_label, fontsize=10, rotation=-90, labelpad=10)
    if plot_lim is not None:
        ax2.set_ylim(ymin=plot_lim[0], ymax=plot_lim[1])

    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines + lines2, labels + labels2, markerscale=1, ncol=2,
              bbox_to_anchor=(1, 1.3), loc=0, frameon=False)
    ax.set_xlabel("Garbage Ratio", fontsize=10)


def _bar_plot(ax, x_data, bar_data, plot_data, title, color, y_labels):
    ax.bar(x_data, bar_data, width=0.6, color=color)
    ax.set_ylabel(y_labels[0], fontsize=10)
    ax.set_title(title, fontsize=16)

    ax2 = ax.twinx()
    ax2.plot(x_data, plot_data,
             color=(246 / 255, 111 / 255, 105 / 255),
             linestyle='-',
             linewidth=3,
             marker='o',
             markersize=5,
             markeredgecolor='blue',
             markerfacecolor='black')

    ax2.set_ylabel(y_labels[1], fontsize=12, rotation=-90, labelpad=10)


def _double_bar(ax, x_data, bar_data, label_data, colors, y_label):
    x = np.arange(len(x_data))
    width = 0.35

    ax.bar(x - width / 2, bar_data[0], width, label=label_data[0], color=colors[0])
    ax.set_ylabel(y_label[0], fontsize=10)
    ax.set_xticks(x)
    ax.set_xticklabels(x_data)

    ax2 = ax.twinx()
    ax2.bar(x + width / 2, bar_data[1], width, label=label_data[1], color=colors[1])
    ax2.set_ylabel(y_label[1], fontsize=10, rotation=-90, labelpad=10)

    # if bar_lim is not None:
    #     ax.set_ylim(ymin=bar_lim[0], ymax=bar_lim[1])

    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines + lines2, labels + labels2, markerscale=1, ncol=2,
              bbox_to_anchor=(1, 1.2), loc=0, frameon=False)
    ax.set_xlabel("Garbage Ratio", fontsize=10)


def _triple_bar(ax, x_data, bar_data, label_data, left_right, colors, y_label):
    x = np.arange(len(x_data))
    width = 0.25
    ax2 = ax.twinx()
    pos = [-width, 0, width]

    for index, bar, label, lr, color in zip(range(3), bar_data, label_data, left_right, colors):

        if lr == 0:
            ax.bar(x + pos[index], bar, width, label=label, color=color)
        else:
            ax2.bar(x + pos[index], bar, width, label=label, color=color)

    ax.set_ylabel(y_label[0], fontsize=10)
    ax2.set_ylabel(y_label[1], fontsize=10)

    ax.set_xticks(x)
    ax.set_xticklabels(x_data)
    # if bar_lim is not None:
    #     ax.set_ylim(ymin=bar_lim[0], ymax=bar_lim[1])

    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines + lines2, labels + labels2, markerscale=1, ncol=2,
              bbox_to_anchor=(1, 1.3), loc=0, frameon=False)
    ax.set_xlabel("Avg I/O & CPU", fontsize=10)


def _percent_bar_draw(ax, x_data, y_data, y_per_name, colors, y_label):
    width = 0.6

    bottom = y_data[0]
    for i in range(len(y_per_name)):
        if i == 0:
            ax.bar(x_data, y_data[i], width, label=y_per_name[i], color=colors[i])
        else:
            ax.bar(x_data, y_data[i], width, bottom=bottom, label=y_per_name[i], color=colors[i])
            for j in range(len(bottom)):
                bottom[j] += y_data[i][j]

    ax.set_ylabel(y_label, fontsize=10)
    ax.set_xlabel("Value Size (KB)", fontsize=10)


def _motivation_draw_1(p1_data, p2_data, p3_data):
    _init_plt_env()

    fig = plt.figure(figsize=(12, 3))
    grid = plt.GridSpec(1, 7, figure=fig)
    ax1 = plt.subplot(grid[0, 0:3])
    ax2 = plt.subplot(grid[0, 3:5])
    ax3 = plt.subplot(grid[0, 5:7])

    _double_plot(ax1, p1_data)

    _double_bar_plot(ax2, p2_data[0], p2_data[1], "Throughput (ops/sec)", 'Data Size (GB)', p2_data[3], p2_data[2],
                     "Read", "Write", "Throughput", p2_data[4], (0, 200), (5000, 18000))

    _double_bar(ax3, p3_data[0], [p3_data[1], p3_data[2]], ["Read", "CPU"],
                p3_data[3], ['Data Size (MB)', 'CPU Time (sec)'])

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/EUROSYS2024 GCoffloading/pic/motivation_1.jpg', dpi=500)


def _motivation_draw_2(p1_data, p2_data, p3_data):
    _init_plt_env()

    fig = plt.figure(figsize=(12, 3))
    grid = plt.GridSpec(1, 7, figure=fig)
    ax1 = plt.subplot(grid[0, 0:3])
    ax2 = plt.subplot(grid[0, 3:5])
    ax3 = plt.subplot(grid[0, 5:7])

    _double_plot(ax1, p1_data)

    _percent_bar_draw(ax2, p2_data[0], p2_data[1], p2_data[2], p2_data[3], p2_data[4])
    ax2.legend(markerscale=1, ncol=2,
               bbox_to_anchor=(1.05, 1.3), loc=0, frameon=False,fontsize=10)

    _percent_bar_draw(ax3, p3_data[0], p3_data[1], p3_data[2], p3_data[3], p3_data[4])
    ax3.legend(markerscale=1, ncol=2,
               bbox_to_anchor=(1.05, 1.3), loc=0, frameon=False,fontsize=10)

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/EUROSYS2024 GCoffloading/pic/motivation_2.jpg', dpi=500)


def _motivation_1():
    # 图1: 0.5 时间
    # 图2:吞吐-io
    # 图3:cpu： jobs-time
    # 图4: 0.4 时间
    # 图5:value GC分解总时间
    # 图6:value GC分解平均时间
    # p1
    qps_file = "/Users/fenghao/Desktop/ssd_ycsb/大数据量/0.5-200M/UPDATE"
    gc_file = "/Users/fenghao/Desktop/ssd_ycsb/大数据量/0.5-200M/GC_TIME"
    ops_res = _get_ops(qps_file, ["READ", "UPDATE"])
    gc_time = _get_gc_time(gc_file)
    x0 = _phrase_time(ops_res[0][0])
    qps_x = [_phrase_time(ops[0]) - x0 for ops in ops_res]
    qps_res = [ops[1] // 10 for ops in ops_res]
    for i in range(len(qps_res) - 1, 0, -1):
        qps_res[i] -= qps_res[i - 1]
    gc_res = _gc_count_at_time(gc_time, [_phrase_time(ops[0]) for ops in ops_res])

    # p2
    gr_params = ["1.0", "0.6", "0.5", "0.4", "0.3"]
    gr_paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M" for param in gr_params]
    gr_throughput_list = [13464, 10857, 9174, 8086, 6011]
    # gr_latency_list = [81.46, 113, 142, 168, 247]
    gr_io_list = [(0, 0), (87, 24), (119, 45), (145, 70), (191, 113)]
    gr_colors = [(144 / 255, 201 / 255, 231 / 255), (33 / 255, 158 / 255, 188 / 255)]
    gr_output_list = [io[1] for io in gr_io_list]
    gr_input_list = [io[0] for io in gr_io_list]

    # p3
    cpu_params = ["1.0", "0.6", "0.5", "0.4", "0.3"]
    cpu_paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M/GC_TIME" for param in cpu_params]
    cpu_time = []
    gc_job_num = []
    cpu_time_avg = []
    for path in cpu_paths:
        result = _get_gc_info(path)
        if len(result) == 0:
            cpu_time.append(0)
            gc_job_num.append(0)
            cpu_time_avg.append(0)
        else:
            cpu_time.append(sum([r[2] for r in result]))
            gc_job_num.append(len(result))
            cpu_time_avg.append(cpu_time[-1] / gc_job_num[-1])
    cpu_colors = [(144 / 255, 201 / 255, 231 / 255),
                  (102 / 255, 205 / 255, 170 / 255)]
    input_avg = [i / n * 1024 if n != 0 else 0 for i, n in zip(gr_input_list, gc_job_num)]

    _motivation_draw_1((qps_x, qps_res, gc_res),
                       (gr_params, gr_throughput_list, gr_output_list, gr_input_list, gr_colors),
                       (cpu_params, input_avg, cpu_time_avg, cpu_colors))


def _motivation_2():
    # p4
    qps_file = "/Users/fenghao/Desktop/ssd_ycsb/大数据量/0.4-200M/UPDATE"
    gc_file = "/Users/fenghao/Desktop/ssd_ycsb/大数据量/0.4-200M/GC_TIME"
    ops_res = _get_ops(qps_file, ["READ", "UPDATE"])
    gc_time = _get_gc_time(gc_file)
    x0 = _phrase_time(ops_res[0][0])
    qps_x = [_phrase_time(ops[0]) - x0 for ops in ops_res]
    qps_res = [ops[1] // 10 for ops in ops_res]
    for i in range(len(qps_res) - 1, 0, -1):
        qps_res[i] -= qps_res[i - 1]
    gc_res = _gc_count_at_time(gc_time, [_phrase_time(ops[0]) for ops in ops_res])

    # p5
    x_data = ['1k', '2k', '3k', '4k']
    paths = ['/Users/fenghao/Desktop/ssd_ycsb/value/' + v_size + '-' + db_size + 'G-200M/GC_TIME' for v_size, db_size in
             zip(x_data, ['33', '61', '91', '115'])]
    time_split_data = [[] for _ in range(5)]
    ops_count_data = [[] for _ in range(3)]
    gc_len = []
    for path in paths:
        gc_stats_result, gc_time_sum, ops_sum = _get_gc_stats_from_file(path)
        time_split = gc_time_sum[:-1]
        time_split.append(gc_time_sum[-1] - sum(time_split))
        gc_len.append(len(gc_stats_result))
        for i in range(5):
            time_split_data[i].append(time_split[i] // 1000000)
        for i in range(3):
            ops_count_data[i].append(ops_sum[i])
    time_splits = ['Read LSM', 'Update LSM', 'Read Blob', 'Write Blob', 'Extra']
    time_splits_to_print = ['Read LSM', 'Read Blob', 'Write Blob']
    time_split_data_to_print = [time_split_data[0], time_split_data[2], time_split_data[3]]
    time_colors = [(219 / 255, 49 / 255, 36 / 255), (252 / 255, 140 / 255, 90 / 255), (255 / 255, 223 / 255, 146 / 255),
                   (140 / 255, 190 / 255, 224 / 255), (75 / 255, 116 / 255, 178 / 255)]

    # p6
    ops_name = ['Read LSM', 'Read Blob', 'Write Back']
    ops_count_avg = [[o / n / 1000 for o, n in zip(ops, gc_len)] for ops in ops_count_data]
    ops_colors = [(69 / 255, 42 / 255, 61 / 255), (68 / 255, 117 / 255, 122 / 255), (183 / 255, 181 / 255, 160 / 255)]

    _motivation_draw_2((qps_x, qps_res, gc_res),
                       ([1, 2, 3, 4], time_split_data_to_print, time_splits_to_print, time_colors, 'Time (sec)'),
                       ([1, 2, 3, 4], ops_count_avg, ops_name, ops_colors, 'Count (k)'))


def main():
    _motivation_2()


if __name__ == '__main__':
    main()
