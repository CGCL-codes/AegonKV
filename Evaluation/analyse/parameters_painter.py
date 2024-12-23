import matplotlib.pyplot as plt
import numpy as np


def init_plt_env():
    plt.rcParams['font.sans-serif'] = ['Times New Roman']
    plt.rcParams['axes.unicode_minus'] = False


def bar_draw(ax, x_data, y_data, title, color, y_label, x_label=None):
    ax.bar(x_data, y_data, width=0.6, color=color)
    ax.set_ylabel(y_label, fontsize=10)
    if x_label is not None:
        ax.set_xlabel(x_label, fontsize=10)
    ax.set_title(title, fontsize=16)


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


def double_plot(ax, x_data, y1_data, y2_data, y1_label, y2_label, title, y_label=None):
    ax.plot(x_data, y1_data,
            label=y1_label,
            color=(246 / 255, 111 / 255, 105 / 255),
            linestyle='-',
            linewidth=3,
            marker='o',
            markersize=5,
            markeredgecolor='blue',
            markerfacecolor='black')

    ax.plot(x_data, y2_data,
            label=y2_label,
            color=(21 / 255, 151 / 255, 165 / 255),
            linestyle='-',
            linewidth=3,
            marker='^',
            markersize=5,
            markeredgecolor='blue',
            markerfacecolor='black')

    ax.legend()
    ax.set_ylabel(y_label, fontsize=12)
    ax.set_title(title, fontsize=16)


def list_plot(ax, x_data, y_data_list, y_label_list, title, label, colors):
    for y_data, y_label, color in zip(y_data_list, y_label_list, colors):
        ax.plot(x_data, y_data,
                label=y_label,
                color=color,
                linestyle='-',
                linewidth=2,
                marker='o',
                markersize=3,
                markeredgecolor=color,
                markerfacecolor=color)

    ax.legend()
    ax.set_ylabel(label, fontsize=12)
    ax.set_title(title, fontsize=16)


def bar_plot(ax, x_data, bar_data, plot_data, bar_color, plot_color, bar_label, plot_label, title):
    ax.bar(x_data, bar_data, width=0.6, color=bar_color)
    ax.set_ylabel(bar_label, fontsize=10)

    ax2 = ax.twinx()
    ax2.plot(x_data, plot_data,
             color=plot_color,
             linestyle='-',
             linewidth=3,
             marker='^',
             markersize=8,
             markeredgewidth=2,
             markeredgecolor=plot_color,
             markerfacecolor='white')
    ax2.set_ylabel(plot_label, fontsize=12, rotation=-90, labelpad=10)
    ax2.set_ylim([400, 1000])

    # ax.legend()
    ax.set_xlabel("Discardable Ratio", fontsize=10)
    ax.set_title(title, fontsize=16)


def draw_discardable(params, throughput_list, latency_list, cpu_list, space_amp_list, io_list, gc_info_list):
    init_plt_env()
    fig = plt.figure(figsize=(12, 8))
    axs = fig.subplots(2, 3)

    # x = [chr(ord('A') + i) for i in range(len(params))]
    x = params
    bar_draw(axs[0, 0], x, throughput_list, "Throughput", (246 / 255, 111 / 255, 105 / 255),
             "Throughput (ops/sec)")

    update_latency_list = []
    read_latency_list = []
    for param_latency in latency_list:
        u = []
        r = []
        for latency in param_latency:
            u.append(latency['update.avg'])
            r.append(latency['read.avg'])
        update_latency_list.append(u)
        read_latency_list.append(r)
    time_range = range(len(update_latency_list[0]))
    colors = [(219 / 255, 49 / 255, 36 / 255), (252 / 255, 140 / 255, 90 / 255), (255 / 255, 223 / 255, 146 / 255),
              (230 / 255, 241 / 255, 243 / 255), (144 / 255, 190 / 255, 224 / 255), (75 / 255, 116 / 255, 178 / 255),
              (78 / 255, 171 / 255, 144 / 255),
              (255 / 255, 194 / 255, 75 / 255)
              ]
    assert len(colors) == len(update_latency_list)
    list_plot(axs[0, 1], time_range, update_latency_list, x, "Update Latency", "Latency (ms)", colors)
    # list_plot(axs[0, 2], time_range, read_latency_list, x, "Read Latency", "Latency (ms)", colors)

    colors = [(237 / 255, 221 / 255, 195 / 255), (238 / 255, 191 / 255, 109 / 255)]
    gc_cpu_list = [cpu[0] for cpu in cpu_list]
    no_cpu_list = [cpu[1] for cpu in cpu_list]
    double_bar_draw(axs[0][2], x, gc_cpu_list, no_cpu_list, "in GC", "not in GC", "Avg CPU", colors, True,
                    ["CPU Usage (%)"])

    bar_draw(axs[1][0], x, space_amp_list, "SPACE AMP", (254 / 255, 179 / 255, 174 / 255), "Space Amplification")

    colors = [(144 / 255, 201 / 255, 231 / 255), (33 / 255, 158 / 255, 188 / 255)]
    output_list = [io[0] for io in io_list]
    input_list = [io[1] for io in io_list]
    double_bar_draw(axs[1][1], x, input_list, output_list, "Read", "Write", "IO", colors, True, ["Data Size (MB)"])

    colors = [(102 / 255, 205 / 255, 170 / 255), (78 / 255, 171 / 255, 144 / 255)]
    gc_times_list = [gc_info[0] for gc_info in gc_info_list]
    gc_duration_list = [gc_info[1] for gc_info in gc_info_list]
    double_bar_draw(axs[1][2], x, gc_times_list, gc_duration_list, "GC Frequency", "Average Duration", "GC INFO",
                    colors, False, ["Frequency (ops)", "Duration (sec)"])

    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/params.jpg', dpi=500)


def draw_batch(params, throughput_list, io_list, gc_list, gc_count_time_percent):
    init_plt_env()
    fig = plt.figure(figsize=(12, 8))
    axs = fig.subplots(2, 2)

    x = params

    bar_draw(axs[0, 0], x, throughput_list, "THROUGHPUT", (246 / 255, 111 / 255, 105 / 255),
             "Throughput (ops/sec)")

    colors = [(144 / 255, 201 / 255, 231 / 255), (33 / 255, 158 / 255, 188 / 255)]
    output_list = [io[0] for io in io_list]
    input_list = [io[1] for io in io_list]
    double_bar_draw(axs[0][1], x, input_list, output_list, "Read", "Write", "IO", colors, True, ["Data Size (MB)"])

    colors = [(102 / 255, 205 / 255, 170 / 255), (78 / 255, 171 / 255, 144 / 255)]
    gc_count = [gc[0] for gc in gc_list]
    gc_time = [gc[1] for gc in gc_list]
    double_bar_draw(axs[1][0], x, gc_count, gc_time, "Count", "Time", "IO", colors, False, ["Count", "Duration (sec)"])

    colors = [(102 / 255, 205 / 255, 170 / 255), (78 / 255, 171 / 255, 144 / 255)]
    gc_times_list = [gc_info[0] for gc_info in gc_info_list]
    gc_duration_list = [gc_info[1] for gc_info in gc_info_list]
    double_bar_draw(axs[1][1], x, gc_times_list, gc_duration_list, "GC Frequency", "Average Duration", "GC INFO",
                    colors, False, ["Frequency (ops)", "Duration (sec)"])

    plt.tight_layout()
    plt.show()


def draw_value(params, throughput_list, cpu_list, latency_list):
    init_plt_env()
    fig = plt.figure(figsize=(8, 3))
    axs = fig.subplots(1, 3)

    x = params

    bar_draw(axs[0], x, throughput_list, "Throughput", (246 / 255, 111 / 255, 105 / 255),
             "Throughput (ops/sec)")

    colors = [(237 / 255, 221 / 255, 195 / 255), (238 / 255, 191 / 255, 109 / 255)]
    gc_cpu_list = [cpu[0] for cpu in cpu_list]
    no_cpu_list = [cpu[1] for cpu in cpu_list]
    double_bar_draw(axs[1], x, gc_cpu_list, no_cpu_list, "in GC", "not in GC", "Avg CPU", colors, True,
                    ["CPU Usage (%)"])

    update_latency_list = []
    for param_latency in latency_list:
        u = []
        for latency in param_latency:
            u.append(latency['update.avg'])
        update_latency_list.append(u)
    time_range = range(len(update_latency_list[0]))
    colors = [(219 / 255, 49 / 255, 36 / 255), (252 / 255, 140 / 255, 90 / 255), (255 / 255, 223 / 255, 146 / 255),
              (78 / 255, 171 / 255, 144 / 255)]
    assert len(colors) == len(update_latency_list)
    list_plot(axs[2], time_range, update_latency_list, x, "Update Latency", "Latency (ms)", colors)

    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/616/pic/value.jpg', dpi=500)


if __name__ == '__main__':
    pass
