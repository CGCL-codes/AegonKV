cmake_minimum_required(VERSION 3.0)

project(gc_kernel)

option(REAL_COMPILE "real compile or just for ide development." OFF)
if (REAL_COMPILE)
    set(CMAKE_C_COMPILER "/home/Vitis/Vitis/2021.1/bin/v++")
    set(CMAKE_CXX_COMPILER "/home/Vitis/Vitis/2021.1/bin/v++")
    message(STATUS "compile for vitis")
endif ()


set(XILINX_XRT /opt/xilinx/xrt)
set(XILINX_VITIS /home/Vitis/Vitis/2021.1/)

#set(TARGET sw_emu)
#set(TARGET hw_emu)
set(TARGET hw)
set(PLATFORM /opt/xilinx/platforms/xilinx_u2_gen3x4_xdma_gc_2_202110_1/xilinx_u2_gen3x4_xdma_gc_2_202110_1.xpfm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

if (NOT REAL_COMPILE)
    file(GLOB SOURCES src/*.c*)
    include_directories(/home/hfeng/tool/Xilinx/Vitis_HLS/2022.2/include)
    include_directories(/home/Vitis_HLS/2021.1/include)
    add_executable(fake_exe ${SOURCES})
endif ()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET} )

add_custom_target(emu_config ALL
        COMMAND emconfigutil --platform ${PLATFORM}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(kernel_hardware ALL
        COMMAND ${CMAKE_CXX_COMPILER} -t ${TARGET} --platform ${PLATFORM} -k gcKernel -I${CMAKE_CURRENT_SOURCE_DIR}/src/ -o gc_kernel.xo -c ${CMAKE_CURRENT_SOURCE_DIR}/src/gc_kernel.cc
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
#        DEPENDS emu_config
)

add_custom_target(gc_kernel ALL
        COMMAND ${CMAKE_CXX_COMPILER} -t ${TARGET} --platform ${PLATFORM} --config ${CMAKE_CURRENT_SOURCE_DIR}/gc.ini -o gc.xclbin -I${CMAKE_CURRENT_SOURCE_DIR}/src/ -l gc_kernel.xo
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
        DEPENDS kernel_hardware
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/gc.xclbin DESTINATION bin)