# AegonKV Evaluation

This repository is used to reproduce all the evaluation results in the AegonKV paper.
This repository is forked from [YCSB-cpp](https://github.com/ls4154/YCSB-cpp.git).

# Compilation
```shell
mkdir build
cd build
# build for tian and AegonKV
cmake -DBIND_ROCKSDB=0 -DBIND_TITANDB=1 -DBIND_DIFFKV=0 ..
make
```

## Running
The evaluation in the paper use three kinds of workload: YCSB, Social Graph, and Twitter Cluster.

### YCSB 
Load data and execute YCSB-A workload against AegonKV.
```shell
./ycsb \
  -load -run -db titandb -P ../workloads/workloada \
  -P ../titandb/aegonkv.properties -s -statistics \
  -p threadcount=16
```

### Social Graph
Generate workload based on the example in Cao's [paper](https://www.usenix.org/conference/fast20/presentation/cao-zhichao).
```shell
$(path to AegonKV)/build/titandb_bench \
  -benchmarks="fillrandom,mixgraph" \
  -use_direct_io_for_flush_and_compaction=true -use_direct_reads=true -cache_size=268435456 \
  -keyrange_dist_a=14.18 -keyrange_dist_b=-2.917 -keyrange_dist_c=0.0164 -keyrange_dist_d=-0.08082 -keyrange_num=30 \
  -value_k=0.923 -value_sigma=226.409 -iter_k=2.517 -iter_sigma=14.236 \
  -mix_get_ratio=0.55 -mix_put_ratio=0.44 -mix_seek_ratio=0.01 \
  -sine_mix_rate_interval_milliseconds=5000 -sine_a=1000 -sine_b=0.000073 -sine_d=80000 \
  -perf_level=2 -reads=420000000 -num=50000000 -key_size=48 -value_size=256 -threads=16 \
  -write_buffer_size=67108864 -max_write_buffer_number=2 -target_file_size_base=8388608 \
  -max_bytes_for_level_base=16777216 -max_bytes_for_level_multiplier=3 \
  -db=./db \
  -save_keys=true \
  -path_save_keys=$(path to save workload trace)
```
Execute Social Graph workload against AegonKV.
```shell
./ycsb \
-load -run -db titandb -P ../workloads/workload_meta \
-P ../titandb/aegonkv.properties -s -statistics \
-p threadcount=16
```

### Twitter Cluster
Download the trace in Yang's [paper](https://www.usenix.org/conference/osdi20/presentation/yang) from the [repository](https://github.com/twitter/cache-trace).

Prepare and execute Cluster-39 workload against AegonKV.

```shell
python ../titandb/workload_prepare.py

./ycsb \
  -load -run -db titandb -P ../workloads/workload_cluster \
  -P ../titandb/aegonkv.properties -s -statistics \
  -p threadcount=16
```

**More specific command scripts can be found in `titandb/script.sh` and `titandb/real-workload.sh`, and configurations can be found under `workloads` folder.**

## Result
Five metrics throughput, tail latency, space usage, compaction I/O, and write stall are used in the paper.

At the end of one run (mey take several hours), you will see the results in the following format (only the last few lines of the output to be used are captured here):
```shell
2024-02-29 09:57:35 3438 sec: 200000000 operations; [READ: Count=99995365 Max=328990.72 Min=9.58 Avg=200.94 90=373.25 99=488.45 99.9=1320.96 99.99=44662.78] [UPDATE: Count=100004635 Max=246022.14 Min=40.35 Avg=330.39 90=508.67 99=641.53 99.9=1645.57 99.99=50692.10]
Run runtime(sec): 3438.48
Run operations success(ops): 200000000
Run operations fail(ops): 0
Run throughput(ops/sec): 58165.3
```
You can get **throughput**, **tail latency** from the results above.

If you turn the option `-statistics` on, then there will also be statistics output. For example, you can find the following entry to get the **compaction I/O** and **write stall** result.
```shell
rocksdb.compact.read.bytes COUNT : 40784350333
rocksdb.compact.write.bytes COUNT : 38094450456
rocksdb.stall.micros COUNT : 0
```
You can get **space usage** with the following command, or use the real-time monitoring script we provide here (`titandb/size_util.py`).
```shell
du -sh ./
```