import pandas

from analyse.time_stats_painter import draw_gc_time_stats


def _get_gc_stats_from_file(path):
    gc_time_data = pandas.read_csv(path)

    time_splits = ['read lsm micros', 'update lsm micros', 'read blob micros', 'write blob micros']
    ops_name = ['read lsm num', 'read blob num', 'write back num']

    gc_stats_result = []
    gc_time_sum = [0 for _ in range(len(time_splits) + 1)]
    ops_sum = [0 for _ in range(len(ops_name))]

    for index, row in gc_time_data.iterrows():
        # if index<20 or index>40:
        #     continue
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


def value_gc():
    x_data = ['1k', '2k', '3k', '4k']
    paths = [
        '/Users/fenghao/Desktop/ASPLOS2024 GCoffloading/motivation_test/ssd_ycsb/value/' + v_size + '-' + db_size + 'G-200M/GC_TIME'
        for v_size, db_size in
        zip(x_data, ['33', '61', '91', '115'])]

    # x_data = ['0.6', '0.5', '0.4', '0.3']
    # paths = ["/Users/fenghao/Desktop/ssd_ycsb/大数据量/" + param + "-200M/GC_TIME" for param in x_data]

    time_split_data = [[] for _ in range(5)]
    ops_count_data = [[] for _ in range(3)]

    for path in paths:
        gc_stats_result, gc_time_sum, ops_sum = _get_gc_stats_from_file(path)
        # print(ops_sum[2]/ops_sum[1])
        time_split = gc_time_sum[:-1]
        time_split.append(gc_time_sum[-1] - sum(time_split))

        for i in range(5):
            # if i == 1:
            #     continue
            time_split_data[i].append(time_split[i] // 1000000 / len(gc_stats_result))
        for i in range(3):
            ops_count_data[i].append(ops_sum[i])
    print(time_split_data)

    # draw_gc_time_stats(x_data, time_split_data, ops_count_data)


def gc_ratio():
    params = ["1.0", "0.7", "0.6", "0.5", "0.45", "0.4", "0.35", "0.25"]
    spaces = [52, 51, 46, 44, 42, 32, 30, 31]
    paths = ["/Users/fenghao/Desktop/ssd_ycsb/垃圾率/" + param + "-" + str(space) + "G/GC_TIME" for param, space in
             zip(params, spaces)]
    duration_data = []
    read_ops = []
    write_back_ops = []

    for path in paths:
        gc_stats_result, gc_time_sum, ops_sum = _get_gc_stats_from_file(path)
        read_ops.append(ops_sum[1])
        write_back_ops.append(ops_sum[2])
        duration_data.append(gc_time_sum[-1] / 1000000)

    for d, r, w in zip(duration_data, read_ops, write_back_ops):
        if r > 0:
            print(d, r, r / d, w / r)
        else:
            print(d, r)


def main():
    value_gc()
    # gc_ratio()


if __name__ == '__main__':
    # main()

    path = '/Users/fenghao/Desktop/S2/motivation/titan/GC_TIME'
    gc_time_data = pandas.read_csv(path)

    time_splits = ['read lsm micros', 'update lsm micros', 'read blob micros', 'write blob micros']

    gc_stats_result = []
    gc_time_sum = [0 for _ in range(len(time_splits) + 1)]

    for index, row in gc_time_data.iterrows():
        duration = int(row['end time']) - int(row['start time'])
        stats = []
        for i in range(len(time_splits)):
            stats.append(int(row[time_splits[i]]) / duration)
            gc_time_sum[i] += int(row[time_splits[i]])
        gc_time_sum[-1] += duration
        gc_stats_result.append(stats)

    time_split = gc_time_sum[:-1]
    time_split.append(gc_time_sum[-1] - sum(time_split))

    time_split_data = []
    for i in range(5):
        time_split_data.append(time_split[i] // 1000000 / len(gc_stats_result))

    print(time_splits)
    print(time_split_data)

    print(gc_stats_result[:20])
