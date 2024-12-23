import time

import matplotlib.pyplot as plt
import numpy as np
import pandas


def init_plt_env():
    plt.rcParams['font.sans-serif'] = ['Times New Roman']
    plt.rcParams['axes.unicode_minus'] = False


def plot_draw(ax, x_data_list, y_data_list, label_list, color_list):
    # ax.scatter(x_data, y_data, c="b", s=1, label="DB Size")
    for x_data, y_data, label, color in zip(x_data_list, y_data_list, label_list, color_list):
        ax.plot(x_data, y_data,
                color=color,
                linestyle='-',
                linewidth=2,
                label=label,
                )
    ax.set_xlabel("Elapsed Seconds", fontsize=12)
    ax.set_ylabel("Size (GB)", fontsize=12)
    ax.set_title("DB Size", fontsize=16)
    ax.tick_params("y", color="b")
    ax.legend()


def phrase_size(file):
    gc_time_data = pandas.read_csv(file)

    t_list = []
    s_list = []
    for index, row in gc_time_data.iterrows():
        t = to_timestamp(row['time'])
        s = to_int_size(row['size'])
        t_list.append(t)
        s_list.append(s)
    return t_list, s_list


def to_timestamp(time_str):
    time_array = time.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    timestamp = int(time.mktime(time_array))
    return timestamp


def to_int_size(size_str):
    num = size_str[:-1]
    unit = size_str[-1]
    if unit == 'G':
        return float(num)
    elif unit == 'K':
        return float(num) / 1024
    else:
        raise


def main():
    x_data_list = []
    y_data_list = []
    label_list = ['infrequent', 'normal', 'frequent', 'heavy']
    color_list = [(219 / 255, 49 / 255, 36 / 255), (252 / 255, 140 / 255, 90 / 255), (255 / 255, 223 / 255, 146 / 255),
                  (144 / 255, 190 / 255, 224 / 255),
                  (75 / 255, 116 / 255, 178 / 255),
                  (78 / 255, 171 / 255, 144 / 255),
                  (255 / 255, 194 / 255, 75 / 255)
                  ]

    params = ['0.6', '0.5', '0.4', '0.3']
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M/SIZE" for param in params]

    # params = ['1k-200M', '2k-61G-200M']
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/value/" + param + "/SIZE" for param in params]

    for path in paths:
        t_list, y_data = phrase_size(path)
        x_data = [i * 30 for i in range(len(t_list))]
        x_data_list.append(x_data)
        y_data_list.append(y_data)

    init_plt_env()
    fig = plt.figure(figsize=(8, 5))
    ax = fig.subplots(1, 1)

    plot_draw(ax, x_data_list, y_data_list, label_list, color_list)

    plt.tight_layout()
    plt.show()
    # plt.savefig('/Users/fenghao/Desktop/ssd_ycsb/pic/db_size.jpg', dpi=500)


if __name__ == '__main__':
    main()
