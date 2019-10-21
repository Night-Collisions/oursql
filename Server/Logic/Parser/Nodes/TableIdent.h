#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_TABLEIDENT_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_TABLEIDENT_H_

#include "Node.h"
class TableIdent : public Node {
    std::string getName() override { return std::string(); }
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_TABLEIDENT_H_
