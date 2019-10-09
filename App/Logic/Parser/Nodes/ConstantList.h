#ifndef OURSQL_APP_LOGIC_PARSER_NODES_CONSTANTLIST_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_CONSTANTLIST_H_

#include <vector>
#include "Constant.h"
#include "Node.h"

class ConstantList : public Node {
   public:
    ConstantList(const std::vector<Node*>& constants)
        : Node(NodeType::constant_list), constants_(constants) {}

    std::vector<Node*> getConstants() { return constants_; }

   private:
    std::vector<Node*> constants_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_CONSTANTLIST_H_
