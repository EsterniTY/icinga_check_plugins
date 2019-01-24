project(libs)

add_library(${PROJECT_NAME}
    src/utils.c
    src/snmp.c
)

add_library(sub::libs ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
