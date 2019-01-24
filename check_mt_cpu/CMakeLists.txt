project(check_mt_cpu)

add_executable(${PROJECT_NAME} main.c check_mt_cpu.c)

target_link_libraries(${PROJECT_NAME} sub::libs)
