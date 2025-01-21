## Social Graph

RocksDB:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db rocksdb -P /home/hfeng/code/YCSB/workloads/workload_meta \
  -P /home/hfeng/code/YCSB/rocksdb/rocksdb.properties -s -statistics \
  -p threadcount=16 \
  >meta_rocksdb 2>&1 &
```
BlobDB:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db rocksdb -P /home/hfeng/code/YCSB/workloads/workload_meta \
  -P /home/hfeng/code/YCSB/rocksdb/rocksdb-blob.properties -s -statistics \
  -p threadcount=16 \
  >meta_rocksdb_blob 2>&1 &
```
DiffKV:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db titandb -P /home/hfeng/code/YCSB/workloads/workload_meta \
  -P /home/hfeng/code/YCSB/titandb/diffkv.properties -s -statistics \
  -p threadcount=16 \
  >meta_diffkv 2>&1 &
```
AegonKV:
```shell
nohup \
  sudo -E \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db titandb -P /home/hfeng/code/YCSB/workloads/workload_meta \
  -P /home/hfeng/code/YCSB/titandb/aegonkv.properties -s -statistics \
  -p threadcount=16 \
  >meta_aegonkv 2>&1 &
```
Titan:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -load -run -db titandb -P /home/hfeng/code/YCSB/workloads/workload_meta \
  -P /home/hfeng/code/YCSB/titandb/titandb.properties -s -statistics \
  -p threadcount=16 \
  >meta_titan 2>&1 &
```


## Twitter Cluster

You need to specify which cluster trace to use in `workload_cluster`.

RocksDB:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -run -db rocksdb -P /home/hfeng/code/YCSB/workloads/workload_cluster \
  -P /home/hfeng/code/YCSB/rocksdb/rocksdb.properties -s -statistics \
  -p threadcount=16 \
  >cluster_rocksdb 2>&1 &
```
BlobDB:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -run -db rocksdb -P /home/hfeng/code/YCSB/workloads/workload_cluster \
  -P /home/hfeng/code/YCSB/rocksdb/rocksdb-blob.properties -s -statistics \
  -p threadcount=16 \
  >cluster_rocksdb-blob 2>&1 &
```
DiffKV:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -run -db titandb -P /home/hfeng/code/YCSB/workloads/workload_cluster \
  -P /home/hfeng/code/YCSB/titandb/diffkv.properties -s -statistics \
  -p threadcount=16 \
  >cluster_diffkv 2>&1 &
```
AegonKV:
```shell
nohup \
  sudo -E \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -run -db titandb -P /home/hfeng/code/YCSB/workloads/workload_cluster \
  -P /home/hfeng/code/YCSB/titandb/aegonkv.properties -s -statistics \
  -p threadcount=16 \
  >cluster_aegonkv 2>&1 &
```
Titan:
```shell
nohup \
  /home/hfeng/code/YCSB/cmake-build-debug-node27/ycsb \
  -run -db titandb -P /home/hfeng/code/YCSB/workloads/workload_cluster \
  -P /home/hfeng/code/YCSB/titandb/titandb.properties -s -statistics \
  -p threadcount=16 \
  >cluster_titan 2>&1 &
```
