project(check_mt_temp)

add_executable(${PROJECT_NAME} main.c check_mt_temp.c)

target_link_libraries(${PROJECT_NAME} sub::libs)
