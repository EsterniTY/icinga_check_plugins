project(check_iftraffic)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -lpcre")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -lcrypto")

add_executable(${PROJECT_NAME}
    main.c
    check_iftraffic.c
    info.c
    file.c
    debug.c
    if_status.c
)

target_link_libraries(${PROJECT_NAME} sub::libs)
