#ifndef OURSQL_APP_LOGIC_PARSER_NODES_EXPRESSION_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_EXPRESSION_H_

#include <vector>
#include "Node.h"

enum class ExprUnit {
    equal = 0,
    not_equal = 0,
    greater = 0,
    greater_eq = 0,
    less = 0,
    less_eq = 0,
    mul = 1,
    div = 1,
    add = 2,
    sub = 2,
};

class Expression : public Node {
   public:
    Expression(Expression* left, ExprUnit expr, Expression* right)
        : Node(NodeType::expression_unit), expr_unit_(expr) {
        childs_.push_back(left);
        childs_.push_back(right);
    }

    ~Expression() override {
        for (auto& c : childs_) {
            delete c;
        }
        childs_.clear();
    }

    std::string getName() override { return std::string(); }

    std::vector<Expression*> childs() { return childs_; }
    ExprUnit exprType() { return expr_unit_; }
    void setExprType(ExprUnit expr) { expr_unit_ = expr; }

   private:
    std::vector<Expression*> childs_;
    ExprUnit expr_unit_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_EXPRESSION_H_
