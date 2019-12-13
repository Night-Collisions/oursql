#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_WITH_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_WITH_H_

#include "Node.h"
class With : public Node {
   public:
    explicit With(bool is_vers)
        : Node(NodeType::with), is_versioned_(is_vers){};
    std::string getName() override { return std::string(); }
    bool isVersioned() { return is_versioned_; }

   private:
    bool is_versioned_;
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_WITH_H_
