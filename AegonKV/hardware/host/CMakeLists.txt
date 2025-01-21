cmake_minimum_required(VERSION 3.0)

project(gc_host)

find_package(OpenCL REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(XILINX_XRT /opt/xilinx/xrt)
set(XILINX_VITIS /home/Vitis/2021.1/)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${OpenCL_INCLUDE_DIRS})
include_directories(${XILINX_XRT}/include)

link_directories(${XILINX_XRT}/lib)

file(GLOB SOURCES src/*.c*)
file(GLOB TEST_SRC test/*.c*)

add_library(${PROJECT_NAME} SHARED ${SOURCES} )
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Debug>:-O0>")
target_link_libraries(${PROJECT_NAME} OpenCL ${OpenCL_LIBRARIES} pthread rt)
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(gc_host_test ${TEST_SRC})
target_link_libraries(gc_host_test ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(DIRECTORY "include/" DESTINATION include FILES_MATCHING PATTERN "*.h*")