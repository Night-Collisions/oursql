#ifndef OURSQL_APP_LOGIC_PARSER_NODES_VARLIST_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_VARLIST_H_

#include <vector>
#include "Node.h"
#include "Variable.h"

class VarList : public Node {
   public:
    VarList(std::vector<Variable*>& vars)
        : Node(NodeType::var_list), vars_(vars) {}

    ~VarList() override {
        for (auto& v : vars_) {
            delete v;
        }
    }

    VarList() : Node(NodeType::var_list) {}

    void push(Variable* var) { vars_.push_back(var); }

    std::vector<Variable*> getVars() { return vars_; }

   private:
    std::vector<Variable*> vars_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_VARLIST_H_
