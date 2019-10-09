project(TestsGenerator_APP)

set(HEADERS
        )

set(DEFINITIONS
        )

init(${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME} main.cpp ${HEADERS} ${DEFINITIONS})

target_link_libraries(${PROJECT_NAME} OurSql_APP_SURSE)

show_compilation_warnings(${PROJECT_NAME})
