/home/hfeng/tool/cmake/bin/cmake --build /home/hfeng/code/titan/cmake-build-debug-node27 --target gc_kernel -- -j 8
# !remember to change sw_emu/hw_emu in gc_driver_test.cc
/home/hfeng/tool/cmake/bin/cmake --build /home/hfeng/code/titan/cmake-build-debug-node27 --target gc_host_test -- -j 8
export XCL_EMULATION_MODE=sw_emu
export XCL_EMULATION_MODE=hw_emu
export EMCONFIG_PATH=/home/hfeng/code/titan/cmake-build-debug-node27/hardware/kernel
sudo rm /home/SmartSSD_data/hfeng/titan/4.blob
sudo -E /home/hfeng/code/titan/cmake-build-debug-node27/hardware/host/gc_host_test

sudo /opt/xilinx/xrt/bin/xbutil examine
sudo /opt/xilinx/xrt/bin/xbutil validate --device 0000:61:00.1
sudo /opt/xilinx/xrt/bin/xbutil reset --device 0000:61:00.1

nohup \
/home/hfeng/tool/cmake/bin/cmake --build /home/hfeng/code/titan/cmake-build-debug-node27 --target gc_kernel -- -j 8 \
> hw_compile.log 2>&1 &