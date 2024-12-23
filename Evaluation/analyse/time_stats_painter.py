import matplotlib.pyplot as plt
import numpy as np


def init_plt_env():
    plt.rcParams['font.sans-serif'] = ['Times New Roman']
    plt.rcParams['axes.unicode_minus'] = False


def percent_bar_draw(ax, x_data, y_data, y_per_name, title, colors, y_label):
    width = 0.6

    bottom = y_data[0]
    for i in range(len(y_per_name)):
        # if y_per_name[i] == 'Update LSM':
            # continue
        if i == 0:
            ax.bar(x_data, y_data[i], width, label=y_per_name[i], color=colors[i])
        else:
            ax.bar(x_data, y_data[i], width, bottom=bottom, label=y_per_name[i], color=colors[i])
            for j in range(len(bottom)):
                bottom[j] += y_data[i][j]

    ax.set_ylabel(y_label, fontsize=12)
    ax.set_title(title, fontsize=16)
    ax.legend()


def draw_gc_time_stats(x_data, time_split_data, ops_count_data):
    init_plt_env()
    fig = plt.figure(figsize=(5, 3))
    ax = fig.subplots(1, 1)

    time_splits = ['Read LSM', 'Update LSM', 'Read Blob', 'Write Blob', 'Extra']
    ops_name = ['Read LSM', 'Read Blob', 'Write Back']

    colors = [(219 / 255, 49 / 255, 36 / 255), (252 / 255, 140 / 255, 90 / 255), (255 / 255, 223 / 255, 146 / 255),
              (140 / 255, 190 / 255, 224 / 255), (75 / 255, 116 / 255, 178 / 255)]
    percent_bar_draw(ax, x_data, time_split_data, time_splits, "Time Cost Decomposition of GC", colors,
                     'Time (sec)')

    colors = [(69 / 255, 42 / 255, 61 / 255), (68 / 255, 117 / 255, 122 / 255), (183 / 255, 181 / 255, 160 / 255)]
    # percent_bar_draw(ax[1], x_data, ops_count_data, ops_name, "Operation counts of GC", colors, 'Ops')

    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/616/pic/time_stats.jpg', dpi=500)
