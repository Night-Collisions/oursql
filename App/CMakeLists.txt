project(OurSql_APP)

set(HEADERS
        Engine/Field.h
        Engine/Table.h
        Engine/Engine.h
        Logic/TableManager.h
        Logic/Parser/Nodes/Node.h
        Logic/Parser/Nodes/Query.h
        Logic/Parser/Nodes/Command.h
        Logic/Parser/Nodes/VarList.h
        Logic/Parser/Nodes/Variable.h
        Logic/Parser/Nodes/Ident.h
        Logic/Parser/ParserManager.h
        )

set(DEFINITIONS
        Engine/Field.cpp
        Engine/Table.cpp
        Logic/TableManager.cpp
        Engine/Engine.cpp
        Logic/Parser/Nodes/Node.cpp
        Logic/Parser/Nodes/Query.cpp
        Logic/Parser/Nodes/Command.cpp
        Logic/Parser/Nodes/VarList.cpp
        Logic/Parser/Nodes/Variable.cpp
        Logic/Parser/Nodes/Ident.cpp
        Logic/Parser/ParserManager.cpp
        )

assemble_parser(Logic/Parser/)

init(${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME} main.cpp ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS})

add_library(${PROJECT_NAME}_SURSE ${HEADERS} ${DEFINITIONS})

show_compilation_warnings(${PROJECT_NAME})
