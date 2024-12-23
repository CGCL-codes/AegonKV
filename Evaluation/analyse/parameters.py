import re
import time

import pandas

from analyse.parameters_painter import draw_discardable, draw_batch, draw_value


def get_cpu_list(paths):
    cpu_paths = [path + "/CPU_UPDATE" for path in paths]
    gc_paths = [path + "/GC_TIME" for path in paths]
    result = []

    for cpu_path, gc_path in zip(cpu_paths, gc_paths):
        gc_time = _get_gc(gc_path, True)
        cpu_info = pandas.read_csv(cpu_path)

        if len(gc_time) == 0:
            result.append((0, cpu_info['cpu%'].mean()))
            continue

        gc_cpu = []
        no_cpu = []
        min_time = gc_time[0][0]
        max_time = gc_time[-1][1]
        gc_num_list = _get_gc_num_list(min_time, max_time, gc_time)
        for _, row in cpu_info.iterrows():
            timestamp = _phrase_time(row['time'])
            cpu_usage = row['cpu%']
            if timestamp < min_time or timestamp > max_time:
                no_cpu.append(cpu_usage)
            else:
                time_point = timestamp - min_time
                if gc_num_list[time_point] > 0:
                    gc_cpu.append(cpu_usage)
                else:
                    no_cpu.append(cpu_usage)
        assert len(gc_cpu) + len(no_cpu) == len(cpu_info)
        result.append((sum(gc_cpu) / len(gc_cpu), sum(no_cpu) / len(no_cpu)))

    return result


def get_gc_count_time_percent(paths, time_list):
    gc_paths = [path + "/GC_TIME" for path in paths]
    result = []

    for t, gc_path in zip(time_list, gc_paths):
        gc_time = _get_gc(gc_path, True)
        start_time = _phrase_time(t[0])
        end_time = _phrase_time(t[1])

        if len(gc_time) == 0:
            result.append([1.0, 0, 0])
            continue

        min_time = gc_time[0][0]
        max_time = gc_time[-1][1]
        gc_num_list = _get_gc_num_list(min_time, max_time, gc_time)

        gc_count_time = [0, 0, 0]
        # end_time = max(end_time, max_time)
        assert min_time - start_time >= 0
        assert end_time - max_time <= 0
        gc_count_time[0] += min_time - start_time
        # gc_count_time[0] += end_time - max_time

        for num in gc_num_list:
            if num > 1:
                gc_count_time[2] += 1
            else:
                gc_count_time[num] += 1

        assert sum(gc_count_time) == end_time - start_time + 1
        result.append([num / sum(gc_count_time) for num in gc_count_time])

    return result


# 差分数组求时间段内每个时间点上正在gc的个数
def _get_gc_num_list(start_time, end_time, gc_time):
    diff_arr = [0 for _ in range(end_time - start_time + 1)]
    for gc in gc_time:
        diff_arr[gc[0] - start_time] += 1
        diff_arr[gc[1] - start_time] -= 1
    res = [diff_arr[0]]
    for i in range(1, len(diff_arr)):
        res.append(res[i - 1] + diff_arr[i])
    assert len(res) == len(diff_arr)
    return res


def get_gc_info_list(paths, timestamps):
    gc_paths = [path + "/GC_TIME" for path in paths]
    result_list = []
    for gc_path, timestamp in zip(gc_paths, timestamps):
        gc_time = _get_gc(gc_path, timestamp)
        if len(gc_time) > 0:
            result_list.append((len(gc_time), sum([t[2] for t in gc_time]) / len(gc_time)))
        else:
            result_list.append((0, 0))
    return result_list


def _get_gc(path, micros=False):
    gc_time_data = pandas.read_csv(path)

    result = []
    for index, row in gc_time_data.iterrows():
        start_time = int(row['start time']) // 1000000 if micros else _phrase_time(row['start time'])
        end_time = int(row['end time']) // 1000000 if micros else _phrase_time(row['end time'])
        duration = end_time - start_time
        result.append((start_time, end_time, duration))

    return result


def _phrase_time(time_str):
    # dt = datetime.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    time_array = time.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    timestamp = int(time.mktime(time_array))
    return timestamp


def get_latency(path):
    latency_list = []
    with open(path, encoding='utf-8') as file:
        for line in file:
            if line[0] == '#' or line.isspace():
                continue
            latency_list.append(_phrase_latency(line, ['read', 'update']))
    del latency_list[:20]
    num_checkpoints = len(latency_list)
    res = latency_list[::num_checkpoints // 20]
    return res


# input example:'[READ: Count=40005816 Max=650117.12 Min=8.98 Avg=94.24 90=173.57 99=218.37 99.9=275.71 99.99=424.19]
# [UPDATE: Count=39994184 Max=634388.48 Min=27.92 Avg=132.03 90=213.63 99=271.87 99.9=557.05 99.99=1020.41]'
def _phrase_latency(latency_str, operations):
    num_re_str = r'=[\d+\.?\d*]*'
    res = {}
    indexes = [j + '.' + i for j in operations for i in ['count', 'max', 'min', 'avg', '90', '99', '99.9', '99.99']]

    re_res = re.findall(num_re_str, latency_str)
    num_re_res = [float(num_str[1:]) for num_str in re_res]
    for i, j in zip(indexes, num_re_res):
        res[i] = j
    return res


def param_discardable_ratio():
    params = ["1.0", "0.7", "0.6", "0.5", "0.45", "0.4", "0.35", "0.25"]
    spaces = [52, 51, 46, 44, 42, 32, 30, 31]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/垃圾率/" + param + "-" + str(space) + "G" for param, space in
             zip(params, spaces)]
    throughput_list = [13464, 13384, 12868, 12638, 13123, 7570, 6842, 6173]  # qps
    io_list = [(0, 0), (211, 792), (2825, 9561), (7343, 17472), (10989, 23124), (27474, 55740),
               (31349, 58452), (46075, 72666)]  # (titandb.gc.bytes.written, titandb.gc.bytes.read) MB

    cpu_list = get_cpu_list(paths)

    gc_info_list = get_gc_info_list(paths,
                                    [True, True, True, True, True, True, True, True])  # [(times, avg duration), ...]

    space_amp_list = [space / 20 for space in spaces]

    latency_path = [path + "/UPDATE" for path in paths]
    latency_list = [get_latency(path) for path in latency_path]
    length = len(latency_list[0])
    for latency in latency_list:
        assert len(latency) == length

    draw_discardable(params, throughput_list, latency_list, cpu_list, space_amp_list, io_list, gc_info_list)


def param_batch_size():
    params = ["1024", "512", "256", "64"]
    spaces = [29, 28, 28, 28]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/batch/" + param + "-" + str(space) + "G" for param, space in
             zip(params, spaces)]
    throughput_list = [7895, 7224, 10837, 7570]
    io_list = [(27119, 54903), (27568, 55665), (27961, 57951), (27473, 56419)]

    gc_list = [(56, 25423), (90, 31837), (175, 35038), (385, 29320)]
    # gc_info_list = get_gc_info_list(paths, [True, True, True, True, True, True, True])

    # todo 不GC、1个GC、1个以上GC的时间占比比较
    time_list = [['2022-06-02 16:05:46', '2022-06-02 18:54:26'], ['2023-06-04 23:11:14', '2023-06-05 02:15:20'],
                 ['2023-06-04 12:12:29', '2023-06-04 15:21:17'], ['2023-05-31 22:47:21', '2023-06-01 01:57:36']]
    gc_count_time_percent = get_gc_count_time_percent(paths, time_list)

    draw_batch(params, throughput_list, io_list, gc_list, gc_count_time_percent)


def param_value():
    params = ['1k', '2k', '4k', '8k']
    spaces = [44, 79, 152, 292]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/value/" + param + "-" + str(space) + "G" for param, space in
             zip(params, spaces)]
    throughput_list = [13248, 10565, 9659, 7746]

    cpu_list = get_cpu_list(paths)

    latency_path = [path + "/UPDATE" for path in paths]
    latency_list = [get_latency(path) for path in latency_path]

    # io_list = [(18550),(),(),()]

    # gc_info_list = get_gc_info_list(paths, [True, True, True, True])

    draw_value(params, throughput_list, cpu_list, latency_list)


def main():
    # param_discardable_ratio()

    # param_batch_size()

    # param_value()


if __name__ == '__main__':
    main()
