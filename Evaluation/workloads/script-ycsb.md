## YCSB

RocksDB:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db rocksdb -P /home/hfeng/code/YCSB/workloads/workloada \
  -P /home/hfeng/code/YCSB/rocksdb/rocksdb.properties -s -statistics \
  -p threadcount=16 \
  >ycsb_rocksdb 2>&1 &
```
BlobDB:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db rocksdb -P /home/hfeng/code/YCSB/workloads/workloada \
  -P /home/hfeng/code/YCSB/rocksdb/rocksdb-blob.properties -s -statistics \
  -p threadcount=16 \
  >ycsb_rocksdb-blob 2>&1 &
```
Titan:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db titandb -P /home/hfeng/code/YCSB/workloads/workloada \
  -P /home/hfeng/code/YCSB/titandb/titandb.properties -s -statistics \
  -p threadcount=16 \
  >ycsb_titan 2>&1 &
```
DiffKV:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db titandb -P /home/hfeng/code/YCSB/workloads/workloada \
  -P /home/hfeng/code/YCSB/titandb/diffkv.properties -s -statistics \
  -p threadcount=16 \
  >ycsb_diffkv 2>&1 &
```
AegonKV:
```shell
nohup \
  sudo -E \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db titandb -P /home/hfeng/code/YCSB/workloads/workloada \
  -P /home/hfeng/code/YCSB/titandb/aegonkv.properties -s -statistics \
  -p threadcount=16 \
  >ycsb_aegonkv 2>&1 &
```

Limit the number of CPU cores to run AegonKV:

```shell
nohup \
  sudo -E \
  taskset -c 44-59 \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db titandb -P /home/hfeng/code/YCSB/workloads/workloada \
  -P /home/hfeng/code/YCSB/titandb/aegonkv.properties -s -statistics \
  -p threadcount=16 \
  >ycsb_aegonkv 2>&1 &
```



## Metrics Monitoring

Real-time monitoring of space usage and CPU usage. You need to specify the pid returned by the nohup command.

```shell
nohup \
  python3 /home/hfeng/code/YCSB/titandb/size_util.py $(pid of nuhup program) \
  >/dev/null 2>&1 &

nohup \
  python3 /home/hfeng/code/YCSB/titandb/cpu_util.py $(pid of nuhup program) \
  >/dev/null 2>&1 &
```
