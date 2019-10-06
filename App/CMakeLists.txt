project(OurSql_APP)

set(HEADERS
        Engine/Field.h
        Engine/Engine.h
        Engine/Table.h
        Logic/TableManager.h
        Logic/Reciever.h
        Core/Exception.h
        Exceptions/QueryException.h
        )

set(DEFINITIONS
        Engine/Field.cpp
        Engine/Engine.cpp
        Engine/Table.cpp
        Logic/TableManager.cpp
        Logic/Reciever.cpp
        Core/Exception.cpp
        Exceptions/QueryException.cpp)

assemble_parser(Logic/Parser/)

init(${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME} main.cpp ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS})

add_library(${PROJECT_NAME}_SURSE ${HEADERS} ${DEFINITIONS})

show_compilation_warnings(${PROJECT_NAME})
