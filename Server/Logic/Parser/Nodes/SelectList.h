#ifndef OURSQL_APP_LOGIC_PARSER_NODES_SELECTLIST_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_SELECTLIST_H_

#include <vector>
#include "Ident.h"
#include "Node.h"

class SelectList : public Node {
   public:
    explicit SelectList(const std::vector<Node*>& list)
        : Node(NodeType::select_list), list_(list) {}

    ~SelectList() override {
        for (auto& l : list_) {
            delete l;
        }
        list_.clear();
    }

    std::vector<Node*> getList() { return list_; }
    std::string getName() override { return std::string(); }

   private:
    std::vector<Node*> list_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_SELECTLIST_H_
