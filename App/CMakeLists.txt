project(OurSql_APP)

set(HEADERS
        Logic/Field.h
        Logic/Table.h
        Logic/TableManager.h
        )

set(DEFINITIONS
        Logic/Field.cpp
        Logic/Table.cpp
        Logic/TableManager.cpp
        main.cpp)

assemble_parser(Logic/Parser/)
add_executable(${PROJECT_NAME} ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS})

show_compilation_warnings(${PROJECT_NAME})
