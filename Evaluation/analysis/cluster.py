import os
import re


def main():
    workloads = ['19', '39', '51']
    folders = ['AegonKV', 'DiffKV', 'BlobDB', 'Titan', 'Titan no GC', 'RocksDB']
    files = ['cluster_aegonkv', 'cluster_diffkv', 'cluster_blobdb', 'cluster_titan', 'cluster_titan', 'cluster_rocksdb']
    base_path='/path to result folder'

    throughput = []
    tail_latency = []
    space = []
    compaction = []
    stall = []

    for workload in workloads:
        cur_throughput = []
        cur_tail_latency = []
        cur_space = []
        cur_compaction = []
        cur_stall = []
        for idx in range(len(files)):
            result_path = os.path.join(base_path, workload, folders[idx], files[idx])
            with open(result_path, 'r') as f:
                content = f.read()

                throughput_match = re.findall(r'Run throughput\(ops/sec\): \d+\.*\d*', content)
                cur_throughput.append(throughput_match[0][len('Run throughput(ops/sec): '):])

                latency_temp = re.findall(r'500000000 operations.+', content)
                latency_match = re.findall(r'99=\d+\.*\d*', latency_temp[0])
                cur_tail_latency.append(latency_match[0][len('99='):])

                compaction_read_match = re.findall(r'rocksdb.compact.read.bytes COUNT : \d+', content)
                compaction_write_match = re.findall(r'rocksdb.compact.write.bytes COUNT : \d+', content)
                cur_compaction.append((int(compaction_read_match[0][len('rocksdb.compact.read.bytes COUNT : '):]) + int(
                    compaction_write_match[0][len('rocksdb.compact.write.bytes COUNT : '):])) // 1000000000)

                stall_match = re.findall(r'rocksdb.stall.micros COUNT : \d+', content)
                cur_stall.append(stall_match[0][len('rocksdb.stall.micros COUNT : '):])

            size_path = os.path.join(base_path, workload, folders[idx], 'SIZE')
            with open(size_path, 'r') as f:
                lines = f.readlines()
                last_line = lines[-1].strip()
                size_match = re.findall(r',.+', last_line)
                cur_space.append(size_match[0][1:-1])

        throughput.append(cur_throughput)
        tail_latency.append(cur_tail_latency)
        space.append(cur_space)
        compaction.append(cur_compaction)
        stall.append(cur_stall)

    for i in range(len(workloads)):
        print(113 * '*')
        print('Cluster-' + workloads[i])
        print('                  %15s %15s %15s %15s %15s %15s' % tuple(folders))
        print("Throughput(ops/s) %15s %15s %15s %15s %15s %15s" % tuple(throughput[i]))
        print("Latency(ms)       %15s %15s %15s %15s %15s %15s" % tuple(tail_latency[i]))
        print("Compaction(GB)    %15s %15s %15s %15s %15s %15s" % tuple(compaction[i]))
        print("Space(GB)         %15s %15s %15s %15s %15s %15s" % tuple(space[i]))
        print("Stall(ms)         %15s %15s %15s %15s %15s %15s" % tuple(stall[i]))


if __name__ == '__main__':
    main()
