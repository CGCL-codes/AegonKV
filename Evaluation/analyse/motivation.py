import matplotlib.pyplot as plt
import numpy as np
import pandas
import time


def init_plt_env():
    plt.rcParams['font.sans-serif'] = ['Times New Roman']
    plt.rcParams['axes.unicode_minus'] = False


def bar_plot(ax, x_data, bar_data, plot_data, title, color, y_labels):
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


def double_bar_plot(ax, x_data, plot_data, plot_label, bar_label, y1_data, y2_data, y1_label, y2_label, title, colors):
    x = np.arange(len(x_data))
    width = 0.35

    ax.bar(x - width / 2, y1_data, width, label=y1_label, color=colors[0])
    ax.set_ylabel(bar_label, fontsize=12)
    ax.bar(x + width / 2, y2_data, width, label=y2_label, color=colors[1])
    ax.legend()

    ax.set_title(title, fontsize=16)
    ax.set_xticks(x)
    ax.set_xticklabels(x_data)
    # ax.set_ylim(ymin=0, ymax=2.0)

    ax2 = ax.twinx()
    ax2.plot(x_data, plot_data,
             color=(246 / 255, 111 / 255, 105 / 255),
             linestyle='-',
             linewidth=3,
             marker='o',
             markersize=5,
             markeredgecolor='blue',
             markerfacecolor='black')
    ax2.set_ylim(ymin=0, ymax=1200)
    ax2.set_ylabel(plot_label, fontsize=12, rotation=-90, labelpad=10)


def double_bar_draw(ax, x_data, y1_data, y2_data, y1_label, y2_label, title, colors, same_y=False, y_label=None):
    x = np.arange(len(x_data))
    width = 0.35

    ax.bar(x - width / 2, y1_data, width, label=y1_label, color=colors[0])
    if y_label is None:
        ax.set_ylabel(y1_label, fontsize=12)
    else:
        ax.set_ylabel(y_label[0], fontsize=12)

    if same_y:
        ax.bar(x + width / 2, y2_data, width, label=y2_label, color=colors[1])
        ax.legend()
    else:
        ax2 = ax.twinx()
        ax2.bar(x + width / 2, y2_data, width, label=y2_label, color=colors[1])
        if y_label is None:
            ax2.set_ylabel(y2_label, fontsize=12, rotation=-90, labelpad=10)
        else:
            ax2.set_ylabel(y_label[1], fontsize=12, rotation=-90, labelpad=10)

        lines, labels = ax.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax2.legend(lines + lines2, labels + labels2)

    ax.set_title(title, fontsize=16)
    ax.set_xticks(x)
    ax.set_xticklabels(x_data)


def count_ratio():
    # params = ["1.0", "0.6", "0.5", "0.4", "0.3"]
    # throughput_list = [13464, 10857, 9174, 8086, 6011]
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M/GC_TIME" for param in params]

    params = ['1k', '2k', '4k', '8k']
    throughput_list = [13248, 10565, 9659, 7746]
    spaces = [44, 79, 152, 292]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/value/" + param + "-" + str(space) + "G/GC_TIME" for param, space in
             zip(params, spaces)]

    # params = ['64-28G', '256-28G', '512-28G', '1024-29G']
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/batch/" + param + "/GC_TIME" for param in
    #          params]

    # params = ["0.7", "0.6", "0.5", "0.4", "0.35"]
    # throughput_list = [13248, 10565, 9659, 7746]
    # spaces = [51, 46, 44, 32, 30]
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/垃圾率/" + param + "-" + str(space) + "G/GC_TIME" for param, space in
    #          zip(params, spaces)]

    cover_ratio = []
    transfer_ratio = []
    # t_ratio=[]
    gc_count = []
    for path in paths:
        result = _get_gc(path)
        gc_count.append(len(result))
        if len(result) == 0:
            cover_ratio.append(0)
            transfer_ratio.append(0)
            # t_ratio.append(0)
        else:
            cover_ratio.append(sum([r[3] for r in result]) / 100000000)
            # cover_ratio.append(sum([r[3] for r in result]) / 40000000)

            transfer_ratio.append(sum([r[4] for r in result]) / sum([r[3] for r in result]))
            # t_ratio.append(sum([r[2] for r in result]) / sum([r[3] for r in result]))
            print(sum([r[4] for r in result]) / sum([r[3] for r in result]))

    colors = [(144 / 255, 201 / 255, 231 / 255), (33 / 255, 158 / 255, 188 / 255)]

    init_plt_env()
    fig = plt.figure(figsize=(8, 4))
    ax = fig.subplots()
    # double_bar_plot(ax, params, gc_count, "GC Count", 'Ratio', cover_ratio, transfer_ratio,
    #                 "cover ratio",
    #                 "transfer ratio",
    #                 "GC Count - Cover/Transfer Ratio",
    #                 colors)

    plt.tight_layout()
    # plt.show()
    # plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/count_ratio_3.jpg', dpi=500)


def _ec_draw(ax, x_data, plot_datas, plot_labels, bar_data, bar_label, y_label, title, colors):
    x = np.arange(len(x_data))
    width = 0.35

    ax.bar(x, bar_data, width, label=bar_label, color=colors[0])

    markers = ['o', '^']
    for d, l, m in zip(plot_datas[:-1], plot_labels[:-1], markers):
        ax.plot(x, d,
                color=(246 / 255, 111 / 255, 105 / 255),
                linestyle='-',
                linewidth=3,
                label=l,
                marker=m,
                markersize=6,
                markeredgecolor='blue',
                markerfacecolor='black')
        ax.set_ylim(ymin=0, ymax=1.5)
    ax.set_ylabel(y_label, fontsize=12)

    ax2 = ax.twinx()
    ax2.plot(x, plot_datas[-1],
             color=(246 / 255, 111 / 255, 105 / 255),
             linestyle='-',
             linewidth=3,
             label=plot_labels[-1],
             marker='x',
             markersize=6,
             markeredgecolor='blue',
             markerfacecolor='black')
    ax2.set_ylabel('frequency', fontsize=12, rotation=-90, labelpad=10)
    ax2.set_ylim(ymin=0, ymax=1200)
    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2)

    ax.set_title(title, fontsize=16)
    ax.set_xticks(x)
    ax.set_xticklabels(x_data)


def eff_cr():
    # params = ["0.6", "0.5", "0.4", "0.3"]
    # throughput_list = [13464, 10857, 9174, 8086, 6011]
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M/GC_TIME" for param in params]
    # garbage_ratio = [0.6, 0.5, 0.4, 0.3]

    # params = ['1k', '2k', '4k', '8k']
    # throughput_list = [13248, 10565, 9659, 7746]
    # spaces = [44, 79, 152, 292]
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/value/" + param + "-" + str(space) + "G/GC_TIME" for param, space in
    #          zip(params, spaces)]
    # garbage_ratio = [0.5, 0.5, 0.5, 0.5]

    # params = ['64', '256', '512', '1024']
    # spaces = [28, 28, 28, 29]
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/batch/" + param + "-" + str(space) + "G/GC_TIME" for param, space in
    #          zip(params, spaces)]
    # garbage_ratio = [0.5, 0.5, 0.5, 0.5]

    params = ['1k', '2k-61G', '3k-91G', '4k-115G']
    x_data = ['1k', '2k', '3k', '4k']
    # spaces = [28, 28, 28, 29]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/value/" + param + "-200M/GC_TIME" for param in
             params]
    garbage_ratio = [0.5, 0.5, 0.5, 0.5]

    cover_ratio = []
    efficiency = []
    frequency = []
    for path in paths:
        result = _get_gc(path)
        cover_ratio.append(sum([r[3] for r in result]) / 100000000)
        # cover_ratio.append(sum([r[3] for r in result]) / 40000000)

        efficiency.append(1 - sum([r[4] for r in result]) / sum([r[3] for r in result]))
        frequency.append(len(result))
        # print(1 - sum([r[4] for r in result]) / sum([r[3] for r in result]))
        # print(frequency[-1])

    colors = [(144 / 255, 201 / 255, 231 / 255), (33 / 255, 158 / 255, 188 / 255)]

    init_plt_env()
    fig = plt.figure(figsize=(8, 4))
    ax = fig.subplots()
    _ec_draw(ax, x_data, [efficiency, garbage_ratio, frequency], ['1-tr', 'G', 'f'], cover_ratio, 'cr', 'ratio',
             'GC Paradigm', colors)

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/para-v.jpg', dpi=500)


def value():
    params = ['1k', '2k-61G', '3k-91G', '4k-115G']
    x_data = ['1k', '2k', '3k', '4k']
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/value/" + param + "-200M/GC_TIME" for param in params]

    frequency = []
    input_list = [119, 225, 337, 455]
    output_list = [45, 88, 135, 181]
    for path in paths:
        result = _get_gc(path)
        frequency.append(len(result))

    init_plt_env()
    fig = plt.figure(figsize=(8, 4))
    ax = fig.subplots()
    colors = [(144 / 255, 201 / 255, 231 / 255), (33 / 255, 158 / 255, 188 / 255)]
    double_bar_plot(ax, x_data, frequency, "Count", 'Data Size (GB)', input_list, output_list, "Read",
                    "Write", "I/O - Frequency", colors)

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/io-f-v.jpg', dpi=500)


def _space_draw(ax, x_data, bar_data, err_data, title, color, y_label):
    error_attri = {'elinewidth': 2, 'ecolor': 'black', 'capsize': 6}
    ax.bar(x_data, bar_data, width=0.6, color=color, yerr=err_data, error_kw=error_attri, )
    ax.set_ylabel(y_label, fontsize=10)
    # ax.set_title(title, fontsize=16)


def _scan_draw(ax, x_data, bar_data, plot_data, title, color, y_labels):
    ax.bar(x_data, bar_data, width=0.6, color=color)
    ax.set_ylabel(y_labels[0], fontsize=10)
    # ax.set_title(title, fontsize=16)
    ax.set_ylim(ymin=0, ymax=1800)

    ax2 = ax.twinx()
    ax2.plot(x_data, plot_data,
             color=(144 / 255, 190 / 255, 224 / 255),
             linestyle='-',
             linewidth=3,
             marker='^',
             markersize=8,
             markeredgecolor=(144 / 255, 190 / 255, 224 / 255),
             markerfacecolor='white',
             markeredgewidth=2)
    ax2.set_ylim(ymin=500, ymax=1000)
    ax2.set_ylabel(y_labels[1], fontsize=10, rotation=-90, labelpad=10)


def space_scan():
    x = ['no gc', 'titan\ndefault', 'diffkv\ndefault', 'diffkv\nlevel merge']
    params = ["1.0", "0.5", "0.35-16G-no", "0.35-21G-l"]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param for param in params]
    throughput_list = [1140, 1324, 1467, 1632]
    latency_list = [858, 734, 665, 596]
    space_list = [3.2, 40 / 20, 31 / 20, 34 / 20]
    space_err_list = [(0, 8 / 20, 5.8 / 20, 6.1 / 20), (0, 8 / 20, 4.1 / 20, 4.8 / 20), ]

    init_plt_env()
    fig = plt.figure(figsize=(5, 7))
    ax = fig.subplots(2, 1)

    _space_draw(ax[0], x, space_list, space_err_list, 'Space Amplification', (246 / 255, 111 / 255, 105 / 255),
                'Amp Ratio')

    _scan_draw(ax[1], x, throughput_list, latency_list, 'Scan Performance', (252 / 255, 140 / 255, 90 / 255),
               ['Throughput (ops/sec)', 'Latency (ms)'])

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/EUROSYS2024 GCoffloading/pic/background.jpg', dpi=500)


def io_latency():
    params = ["1.0", "0.6", "0.5", "0.4", "0.3"]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M" for param in params]
    # throughput_list = [13464, 13384, 12868, 12638, 13123, 7570, 6842, 6173]
    latency_list = [81.46, 113, 142, 168, 247]
    io_list = [(0, 0), (87, 24), (119, 45), (145, 70), (191, 113)]
    colors = [(144 / 255, 201 / 255, 231 / 255), (33 / 255, 158 / 255, 188 / 255)]
    output_list = [io[1] for io in io_list]
    input_list = [io[0] for io in io_list]

    init_plt_env()
    fig = plt.figure(figsize=(8, 4))
    ax = fig.subplots()
    double_bar_plot(ax, params, latency_list, "Latency (ms)", 'Data Size (GB)', input_list, output_list, "Read",
                    "Write",
                    "I/O - Latency",
                    colors)

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/io-l.jpg', dpi=500)


def m_cpu():
    params = ["1.0", "0.6", "0.5", "0.4", "0.3"]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M/GC_TIME" for param in params]
    cpu_time = []
    gc_kv_num_list = []
    for path in paths:
        result = _get_gc(path)
        if len(result) == 0:
            cpu_time.append(0)
            gc_kv_num_list.append(0)
        else:
            cpu_time.append(sum([r[2] for r in result]))
            gc_kv_num_list.append(sum([r[3] for r in result]))

    init_plt_env()
    fig = plt.figure(figsize=(8, 4))
    ax = fig.subplots()
    bar_plot(ax, params, cpu_time, gc_kv_num_list, "CPU", (102 / 255, 205 / 255, 170 / 255),
             ['CPU Time(Sec)', 'GC KV num'])

    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/cpu-t.jpg', dpi=500)


def get_gc_info_list(paths):
    # gc_paths = [path + "/GC_TIME" for path in paths]
    result_list = []
    for gc_path in paths:
        gc_time = _get_gc(gc_path)
        if len(gc_time) > 0:
            result_list.append((len(gc_time), sum([t[2] for t in gc_time]) / len(gc_time)))
        else:
            result_list.append((0, 0))
    return result_list


def _get_gc(path, micros=True):
    gc_time_data = pandas.read_csv(path)

    result = []
    for index, row in gc_time_data.iterrows():
        start_time = int(row['start time']) // 1000000 if micros else _phrase_time(row['start time'])
        end_time = int(row['end time']) // 1000000 if micros else _phrase_time(row['end time'])
        duration = end_time - start_time
        result.append((start_time, end_time, duration, int(row['read blob num']), int(row['write back num'])))

    return result


def _phrase_time(time_str):
    # dt = datetime.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    time_array = time.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    timestamp = int(time.mktime(time_array))
    return timestamp


def m_efficiency():
    x_data = ['1k', '2k', '4k', '8k']
    paths = ['/Users/fenghao/Desktop/ssd_ycsb/value/' + v_size + '-' + db_size + 'G/GC_TIME' for v_size, db_size in
             zip(x_data, ['44', '79', '152', '292'])]
    gc_info_list = get_gc_info_list(paths)
    init_plt_env()
    fig = plt.figure(figsize=(6, 4))
    ax = fig.subplots()

    colors = [(102 / 255, 205 / 255, 170 / 255), (78 / 255, 171 / 255, 144 / 255)]
    gc_times_list = [gc_info[0] for gc_info in gc_info_list]
    gc_duration_list = [gc_info[1] for gc_info in gc_info_list]
    double_bar_draw(ax, x_data, gc_times_list, gc_duration_list, "GC Frequency", "Average Duration", "GC INFO",
                    colors, False, ["Frequency (ops)", "Duration (sec)"])
    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/eff.jpg', dpi=500)


def main():
    space_scan()


if __name__ == '__main__':
    main()
