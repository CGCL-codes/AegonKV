



/usr/bin/cmake --build /home/hfeng/code/titan/cmake-build-debug-node25 --target gc_kernel -- -j 8
# !remember to change sw_emu/hw_emu in gc_driver_test.cc
/usr/bin/cmake --build /home/hfeng/code/titan/cmake-build-debug-node25 --target gc_host -- -j 8
/usr/bin/cmake --build /home/hfeng/code/titan/cmake-build-debug-node25 --target gc_host_test -- -j 8

export XCL_EMULATION_MODE=sw_emu
export XCL_EMULATION_MODE=hw_emu
export EMCONFIG_PATH=/home/hfeng/code/titan/cmake-build-debug-node25/hardware/kernel
rm /home/SmartSSD_Data/hfeng/titan/4.blob
export XRT_INI_PATH=/home/hfeng/code/titan/hardware/kernel/xrt.ini
/home/hfeng/code/titan/cmake-build-debug-node25/hardware/host/gc_host_test

xbutil examine
xbutil validate --device 0000:3e:00.1
xbutil reset --device 0000:3e:00.1

nohup \
/usr/bin/cmake --build /home/hfeng/code/titan/cmake-build-debug-node25 --target gc_kernel -- -j 8 \
> hw_compile.log 2>&1 &


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