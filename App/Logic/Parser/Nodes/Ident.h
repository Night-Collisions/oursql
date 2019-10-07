#ifndef OURSQL_APP_LOGIC_PARSER_NODES_IDENT_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_IDENT_H_

#include <string>
#include "Node.h"

class Ident : public Node {
   public:
    explicit Ident(const std::string& name) : Node(NodeType::id), name_(name) {}
    explicit Ident(const std::string& table_name, const std::string& name)
        : Node(NodeType::id), table_name_(table_name), name_(name) {}

    std::string getName() { return name_; }

   private:
    std::string name_;
    std::string table_name_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_IDENT_H_
