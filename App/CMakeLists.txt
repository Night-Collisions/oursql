project(OurSql_APP)

set(HEADERS
        )

set(DEFINITIONS
        main.cpp
        Logic/Table.cpp Logic/Table.h Logic/Field.cpp Logic/Field.h Logic/TableManager.cpp Logic/TableManager.h)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(SCANNER ${CMAKE_CURRENT_SOURCE_DIR}/Logic/Parser/scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
BISON_TARGET(PARSER ${CMAKE_CURRENT_SOURCE_DIR}/Logic/Parser/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

ADD_FLEX_BISON_DEPENDENCY(SCANNER PARSER)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME} ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS})

show_compilation_warnings(${PROJECT_NAME})
