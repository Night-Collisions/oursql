#ifndef OURSQL_APP_LOGIC_PARSER_NODES_SELECTLIST_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_SELECTLIST_H_

#include <vector>
#include "Ident.h"
#include "Node.h"

class SelectList : public Node {
   public:
    SelectList(const std::vector<Ident>& list)
        : Node(NodeType::select_list), list_(list) {}

    std::vector<Ident> getList() { return list_; }
    std::string getName() override { return std::string(); }

   private:
    std::vector<Ident> list_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_SELECTLIST_H_
