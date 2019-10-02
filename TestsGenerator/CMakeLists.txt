project(TestsGenerator_APP)

set(HEADERS
        )

set(DEFINITIONS
        )

assemble_parser(../App/Logic/Parser/)

add_executable(${PROJECT_NAME} main.cpp ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS})

target_link_libraries(${PROJECT_NAME} OurSql_APP_SURSE)

show_compilation_warnings(${PROJECT_NAME})
