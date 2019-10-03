project(OurSql_APP)

set(HEADERS
        Engine/Field.h
        Engine/Table.h
        Logic/TableManager.h
        Exceptions/QueryException.h
        )

set(DEFINITIONS
        Engine/Field.cpp
        Engine/Table.cpp
        Logic/TableManager.cpp
        Engine/Engine.cpp
        Engine/Engine.h
        Exceptions/QueryException.cpp)

assemble_parser(Logic/Parser/)

init(${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME} main.cpp ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS} Logic/Reciever.cpp Logic/Reciever.h Exceptions/QueryException.h Exceptions/QueryException.cpp ../Tests/Test.cpp)

add_library(${PROJECT_NAME}_SURSE ${HEADERS} ${DEFINITIONS})

show_compilation_warnings(${PROJECT_NAME})
