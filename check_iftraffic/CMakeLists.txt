project(check_iftraffic)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -lpcre")

add_executable(${PROJECT_NAME}
    main.c
    check_iftraffic.c
    ifsnmp.c
    info.c
    file.c
    debug.c
)

target_link_libraries(${PROJECT_NAME} sub::libs)
