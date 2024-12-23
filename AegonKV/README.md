# AegonKV: A High Bandwidth, Low Tail Latency, and Low Storage Cost KV-Separated LSM Store with SmartSSD-based GC Offloading
AegonKV is built on Titan, and this repository is forked from [tikv/titan](https://github.com/tikv/titan.git).
## Dependencies
Hardware. We use a server with hardware dependencies shown below.
```
[CPU]: 2 x 18-core 2.20GHz Intel Xeon Gold 5220 with two-way hyper-threading
[DRAM]: 1 x 64 GB 2666 MHz DDR4 DRAM
[SSD]: 1 x Samsung SmartSSD® [REQUIRED!]
```
Software. Before compilation, it's essential that you have already installed software dependencies shown below.
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
## Compilation

Follow three steps to build AegonKV: build RocksDB dependency (required by Titan), build AegonKV software side target, and build AegonKV FPGA hardware side target.

*Before building hardware target, make sure the SmartSSD environment is configured correctly. Some useful test commands are in [hardware/host/test_commands.sh](./hardware/host/test_commands.sh).*

```shell
mkdir -p build
cd build
cmake -DROCKSDB_DIR=$(pwd)/../lib/rocksdb-6.29.tikv -DREAL_COMPILE=1 -DCMAKE_BUILD_TYPE=Debug ..
# build RocksDB
make -j rocksdb
# build software
make -j titan

# build hardware (it may takes several hours)
cd ..
nohup cmake --build $(pwd)/build --target gc_kernel -- -j 8 > hw_compile.log 2>&1 &
```
