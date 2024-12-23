import matplotlib.pyplot as plt
import numpy as np


def init_plt_env():
    plt.rcParams['font.sans-serif'] = ['Times New Roman']
    plt.rcParams['axes.unicode_minus'] = False


def qps_draw(ax, x_data, y_data):
    ax.scatter(x_data, y_data, c="b", s=1, label="Throughput")
    ax.set_xlabel("Elapsed Seconds", fontsize=12)
    ax.set_ylabel("Throughput (ops/sec)", fontsize=12)
    ax.tick_params("y", color="b")
    # ax.set_ylim([0, 10000])


def qps_draw_line(ax, x_list, y_data_list,label_list):
    for x_data, y_data,label in zip(x_list, y_data_list,label_list):
        ax.plot(x_data, y_data,label=label)
    ax.set_xlabel("Elapsed Seconds", fontsize=12)
    ax.set_ylabel("Throughput (ops/sec)", fontsize=12)
    ax.tick_params("y", color="b")
    # ax.set_ylim([0, 10000])


def cpu_draw(ax, x_data, y_data):
    ax.scatter(x_data, y_data, c="b", s=1, label="CPU Usage")
    ax.set_xlabel("Elapsed Seconds", fontsize=12)
    ax.set_ylabel("CPU Usage (%)", fontsize=12)
    ax.tick_params("y", color="b")


def gc_draw(ax, x_data, y_data):
    ax.scatter(x_data, y_data, c="r", s=1, label="GC Count")
    ax.set_ylabel("GC Count (ops)", fontsize=12, rotation=-90, labelpad=12)
    ax.set_yticks(np.arange(0, max(y_data) + 3, 1))
    ax.tick_params("y", color="r")


def draw_qps_gc(x, qps_res, gc_res):
    init_plt_env()
    fig = plt.figure(figsize=(8, 4))
    ax1 = fig.subplots()
    qps_draw(ax1, x, qps_res)

    ax2 = ax1.twinx()
    gc_draw(ax2, x, gc_res)

    lines, labels = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2, markerscale=5, loc='best')

    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/qps.jpg', dpi=500)


def draw_qps_list(x_list, qps_res_list,label_list):
    init_plt_env()
    fig = plt.figure(figsize=(8, 4))
    ax = fig.subplots()
    qps_draw_line(ax, x_list, qps_res_list,label_list)

    ax.legend()
    plt.tight_layout()
    # plt.show()
    plt.savefig('/Users/fenghao/Desktop/qps.jpg', dpi=500)


def draw_throughput_time_gc(x, qps_res, gc_res):
    init_plt_env()
    fig = plt.figure(figsize=(4, 3))
    ax = fig.subplots()
    qps_list = [0, 0]
    time_list = [0, 0]
    for q, t in zip(qps_res, gc_res):
        if t != 0:
            t = 1
        qps_list[t] += q
        time_list[t] += 1
    qps_list[0] /= time_list[0]
    qps_list[1] /= time_list[1]
    time_sum = time_list[0] + time_list[1]
    time_list[0] = time_list[0] / time_sum * 100
    time_list[1] = time_list[1] / time_sum * 100

    x_data = ["Not in GC", "During GC"]

    x = np.arange(len(x_data))
    width = 0.30

    ax.bar(x - width / 2, qps_list, width, label="Throughput", color=(246 / 255, 111 / 255, 105 / 255))
    ax.set_ylabel("Throughput (ops/sec)", fontsize=12)
    ax.set_ylim(ymin=0, ymax=10000)

    ax2 = ax.twinx()
    ax2.bar(x + width / 2, time_list, width, label="Time", color=(252 / 255, 140 / 255, 90 / 255))
    ax2.set_ylabel("Time Percentage (%)", fontsize=12, rotation=-90, labelpad=13)
    ax2.set_ylim(ymin=0, ymax=60)

    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2, markerscale=7, ncol=2,
               bbox_to_anchor=(1, 1.2), loc=0, frameon=False, fontsize=11)

    ax.set_xticks(x)
    ax.set_xticklabels(x_data)

    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/motivation-throughput.jpg', dpi=500)


def draw_cpu_gc(x, qps_res, gc_res):
    init_plt_env()
    fig, ax1 = plt.subplots()
    cpu_draw(ax1, x, qps_res)

    ax2 = ax1.twinx()
    gc_draw(ax2, x, gc_res)

    lines, labels = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2, markerscale=5, loc='best')

    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/cpu.jpg', dpi=500)


if __name__ == '__main__':
    pass
