project(OurSql_APP)

set(HEADERS
        Engine/Field.h
        Engine/Table.h
        Logic/TableManager.h
        )

set(DEFINITIONS
        Engine/Field.cpp
        Engine/Table.cpp
        Logic/TableManager.cpp
        main.cpp)

assemble_parser(Logic/Parser/)
add_executable(${PROJECT_NAME} ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS})

show_compilation_warnings(${PROJECT_NAME})
