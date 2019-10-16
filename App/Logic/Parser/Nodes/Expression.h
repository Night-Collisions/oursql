#ifndef OURSQL_APP_LOGIC_PARSER_NODES_EXPRESSION_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_EXPRESSION_H_

#include <vector>
#include "Constant.h"
#include "Node.h"

enum class ExprUnit : unsigned int {
    equal,
    not_equal,
    greater,
    greater_eq,
    less,
    less_eq,
    and_,
    or_,
    not_,
    mul,
    div,
    add,
    sub,
    value,
    Count
};

class Expression : public Node {
   public:
    Expression(Expression* left, ExprUnit expr, Expression* right)
        : Node(NodeType::expression_unit), expr_unit_(expr) {
        childs_.push_back(left);
        childs_.push_back(right);
    }

    explicit Expression(Node* val)
        : Node(NodeType::expression_unit),
          expr_unit_(ExprUnit::value),
          value_(val) {
        childs_.push_back(nullptr);
        childs_.push_back(nullptr);
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

    Node* getConstant() { return value_; }
    void setConstant(Node* val) {
        delete value_;
        value_ = val;
    }

   private:
    std::vector<Expression*> childs_;
    ExprUnit expr_unit_;
    Node* value_{nullptr};
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_EXPRESSION_H_
