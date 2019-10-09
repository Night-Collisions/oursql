#ifndef OURSQL_APP_LOGIC_PARSER_NODES_IDENTLIST_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_IDENTLIST_H_

#include <vector>
#include "Ident.h"
#include "Node.h"

class IdentList : public Node {
   public:
    explicit IdentList(const std::vector<Ident*>& idents)
        : Node(NodeType::ident_list), idents_(idents) {}

    std::vector<Ident*> getIdents() { return idents_; }

   private:
    std::vector<Ident*> idents_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_IDENTLIST_H_
