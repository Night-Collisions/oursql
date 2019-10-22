project(OurSQL_Server)

set(HEADERS
        Our.h
        Server.h
        Engine/Column.h
        Engine/Engine.h
        Engine/Table.h
        Engine/Engine.h
        Engine/Block.h
        Engine/Cursor.h
        Core/Exception.h
        Core/DataType.h
        Logic/Parser/ParserManager.h
        Logic/Parser/Nodes/Node.h
        Logic/Parser/Nodes/Query.h
        Logic/Parser/Nodes/VarList.h
        Logic/Parser/Nodes/Variable.h
        Logic/Parser/Nodes/Ident.h
        Logic/Parser/Nodes/SelectList.h
        Logic/Parser/Nodes/IntConstant.h
        Logic/Parser/Nodes/RealConstant.h
        Logic/Parser/Nodes/IdentList.h
        Logic/Parser/Nodes/ConstantList.h
        Logic/Parser/Nodes/TextConstant.h
        Logic/Parser/Nodes/Constant.h
        Logic/Parser/ExpressionParser/Resolver.h
        Logic/Parser/RelationalOperationsParser/Join.h
        Logic/Parser/Nodes/RelExpr.h
        Logic/Parser/RelationalOperationsParser/Intersect.h
        Logic/Parser/RelationalOperationsParser/Union.h
        Logic/Parser/Nodes/NullConstant.h
        Logic/Parser/ParserManager.h
        Core/DataType.h
        Logic/Parser/Nodes/Expression.h
        Logic/QueryManager.h
        )

set(DEFINITIONS
        Our.cpp
        Server.cpp
        Engine/Column.cpp
        Engine/Engine.cpp
        Engine/Table.cpp
        Engine/Block.cpp
        Engine/Cursor.cpp
        Engine/Value.h
        Core/Exception.cpp
        Core/DataType.cpp
        Engine/Engine.cpp
        Logic/Parser/ParserManager.cpp
        Logic/Parser/Nodes/Node.cpp
        Logic/Parser/Nodes/Query.cpp
        Logic/Parser/Nodes/VarList.cpp
        Logic/Parser/Nodes/ConstantList.cpp
        Logic/Parser/Nodes/Variable.cpp
        Logic/Parser/Nodes/Ident.cpp
        Logic/Parser/Nodes/SelectList.cpp
        Logic/Parser/RelationalOperationsParser/Join.cpp
        Logic/Parser/RelationalOperationsParser/Intersect.cpp
        Logic/Parser/RelationalOperationsParser/Union.cpp
        Logic/Parser/Nodes/IntConstant.cpp
        Logic/Parser/Nodes/RealConstant.cpp
        Logic/Parser/Nodes/TextConstant.cpp
        Logic/Parser/Nodes/RelExpr.cpp
        Logic/Parser/Nodes/IdentList.cpp
        Logic/Parser/ParserManager.cpp
        Logic/Parser/Nodes/Constant.cpp
        Logic/Parser/Nodes/Expression.cpp
        Logic/Parser/Nodes/NullConstant.cpp
        Logic/Parser/ExpressionParser/Resolver.cpp
        Core/DataType.cpp
        Logic/QueryManager.cpp
        )

assemble_parser(Logic/Parser/)

init(${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME} main.cpp ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS} )

add_library(${PROJECT_NAME}_SURSE ${HEADERS} ${DEFINITIONS} ${FLEX_SCANNER_OUTPUTS})

show_compilation_warnings(${PROJECT_NAME})
