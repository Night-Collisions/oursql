#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_RELEXPR_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_RELEXPR_H_

#include <vector>
#include "Expression.h"
#include "Ident.h"
#include "Node.h"
enum class RelOperNodeType : unsigned int {
    join,
    inner_join,
    outer_join,
    full_join,
    cross_join,
    left_join,
    right_join,
    union_,
    intersect,
    table_ident,
    Count
};

class RelExpr : public Node {
   public:
    RelExpr(RelExpr* left, RelExpr* right, RelOperNodeType type,
            Expression* cond_expr)
        : Node(NodeType::relational_oper_expr),
          rel_oper_type_(type),
          on_expr_(cond_expr) {
        childs_.push_back(left);
        childs_.push_back(right);
    }

    RelExpr(Node* table_name, const std::string& alias_name)
        : Node(NodeType::relational_oper_expr),
          table_id_(static_cast<Ident*>(table_name)),
          rel_oper_type_(RelOperNodeType::table_ident) {
        childs_.push_back(nullptr);
        childs_.push_back(nullptr);
    }

    ~RelExpr() override {
        for (auto& c : childs_) {
            delete c;
        }
        delete table_id_;
        delete on_expr_;
        childs_.clear();
    }

    RelOperNodeType getRelOperType() { return rel_oper_type_; }

    std::vector<RelExpr*> childs() { return childs_; }

    std::string getName() override {
        return (table_id_) ? (alias_name_) : (table_id_->getName());
    }
    std::string getAlias() { return alias_name_; }

    Expression* getOnExpr() { return on_expr_; }

   private:
    RelOperNodeType rel_oper_type_;
    std::vector<RelExpr*> childs_;
    Expression* on_expr_{nullptr};
    Ident* table_id_{nullptr};
    std::string alias_name_{""};
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_RELEXPR_H_
