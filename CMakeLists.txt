project (check_mikrotik)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -L/usr/lib -lnetsnmp")
# set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -L/usr/lib -lnetsnmpagent -lnetsnmpmibs")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -s")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

add_subdirectory(libs)
add_subdirectory(check_mt_temp)
add_subdirectory(check_mt_cpu)
add_subdirectory(check_iftraffic)
