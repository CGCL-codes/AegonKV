# AegonKV
AegonKV is a "three-birds-one-stone" solution that comprehensively enhances the throughput, tail latency, and space usage of KV separated systems.

This project provides implementation source code of AegonKV and evaluation scripts.

AegonKV is accepted by FAST'25, the detailed paper can be viewed [here](https://www.usenix.org/conference/fast25/presentation/duan).

**Publications**: Zhuohui Duan, Hao Feng, Haikun Liu, Xiaofei Liao, Hai Jin, Bangyu Li. AegonKV: A High Bandwidth, Low Tail Latency, and Low Storage Cost KV-Separated LSM Store with SmartSSD-based GC Offloading.



## Build AegonKV

Folder [AegonKV](./AegonKV) contains the source code of AegonKV, you can build AegonKV according to the instructions below (also in [AegonKV/README.md](./AegonKV/README.md)).
### Dependencies
**Hardware**. We use a server with hardware dependencies shown below.
```
[CPU]: 2 x 18-core 2.20GHz Intel Xeon Gold 5220 with two-way hyper-threading
[DRAM]: 1 x 64 GB 2666 MHz DDR4 DRAM
[SSD]: 1 x Samsung SmartSSD® [REQUIRED!]
```
In particular, the SmartSSD environment can be verified with the following command.
```shell
xbutil examine # Get PCIe address of SmartSSD
xbutil validate --device $(PCIe address of SmartSSD) # Verify SmartSSD
xbutil reset --device $(PCIe address of SmartSSD) # Most of the time you can fix validation failures by resetting the environment
```

**Software**. Before compilation, it's essential that you have already installed software dependencies shown below.
```
GCC (9.4.0, https://gcc.gnu.org)
CMake (3.25.6, https://cmake.org)
zlib (1.3.1, https://zlib.net)
gflags (2.2.2, https://github.com/gflags/gflags)
Vitis (2021.1, https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vitis/archive-vitis.html)
Xilinx XRT (2021.1, https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/alveo/smartssd.html)
SmartSSD Development Target Platform (2021.1, https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/alveo/smartssd.html)
Python (3.8.10, https://www.python.org)
```
### Compilation

Follow three steps to build AegonKV: build RocksDB dependency (required by Titan), build AegonKV software side target, and build AegonKV FPGA hardware side target.

*Before building hardware target, make sure the SmartSSD environment is configured correctly. Some useful test commands are in [AegonKV/hardware/host/test_commands.sh](AegonKV/hardware/host/test_commands.sh).*

```shell
cd AegonKV
mkdir -p build
cd build
cmake -DROCKSDB_DIR=$(pwd)/../lib/rocksdb-6.29.tikv -DREAL_COMPILE=1 -DCMAKE_BUILD_TYPE=Debug ..
# build RocksDB dependency
make -j rocksdb
# build software
make -j titan

# build hardware (it may takes several hours)
cd ..
nohup cmake --build $(pwd)/build --target gc_kernel -- -j 8 > hw_compile.log 2>&1 &
```



## Run Evaluations
Folder [Evaluation](./Evaluation) contains evaluation program and configurations, you can build and reproduce evaluation results according to the instructions below (also in [Evaluation/README.md](./Evaluation/README.md)).
### Compilation
```shell
cd Evaluation
mkdir build
cd build
# build for tian and AegonKV
cmake -DBIND_ROCKSDB=0 -DBIND_TITANDB=1 -DBIND_DIFFKV=0 ..
make
```

### Running
The evaluation in the paper use three kinds of workload: YCSB, Social Graph, and Twitter Cluster.

#### Configurations

Running a test requires the use of two configuration files, the workload configuration and the system configuration.

```
1. You can find the workload configuration in the following file:
YCSB: Evaluation/workloads/workload{a-f}
Social Graph: Evaluation/workloads/workload_meta
Twitter Cluster: Evaluation/workloads/workload_cluster # You need to modify the last configuration item to choose which cluster to use.

2. You can find the system configuration in the following file:
AegonKV: Evaluation/titandb/aegonkv.properties
DiffKV: Evaluation/titandb/diffkv.properties
Titan: Evaluation/titandb/titandb.properties
RocksDB: Evaluation/rocksdb/rocksdb.properties
BlobDB: Evaluation/rocksdb/rocksdb-blob.properties
```

#### YCSB
Load data and execute YCSB-A workload against AegonKV.
```shell
./ycsb \
  -load -run -db titandb -P ../workloads/workloada \
  -P ../titandb/aegonkv.properties -s -statistics \
  -p threadcount=16
```

#### Social Graph
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

#### Twitter Cluster
Download the trace in Yang's [paper](https://www.usenix.org/conference/osdi20/presentation/yang) from the [repository](https://github.com/twitter/cache-trace).

Prepare and execute Cluster-39 workload against AegonKV.

```shell
python ../titandb/workload_prepare.py

./ycsb \
  -load -run -db titandb -P ../workloads/workload_cluster \
  -P ../titandb/aegonkv.properties -s -statistics \
  -p threadcount=16
```

**More specific command scripts can be found in [Evaluation/workloads/script-ycsb.md](Evaluation/workloads/script-ycsb.md) and [Evaluation/workloads/script-production.md](Evaluation/workloads/script-production.md).**

### Result Analysis
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
You can get **space usage** with the following command, or use the real-time monitoring script we provide here ([Evaluation/titandb/size_util.py](Evaluation/titandb/size_util.py)).
```shell
du -sh ./
```

You can also use the scripts under [Evaluation/analysis](Evaluation/analysis) to automatically get a summary of the data that reproduces the results of the paper.
