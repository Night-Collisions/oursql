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
        if (left->getNodeType() == NodeType::constant) {
            left->setPropagational(true);
        }
        if (right->getNodeType() == NodeType::constant) {
            right->setPropagational(true);
        }
        childs_.push_back(left);
        childs_.push_back(right);
    }

    explicit Expression(Node* val)
        : Node(NodeType::expression_unit),
          expr_unit_(ExprUnit::value),
          value_(val) {
        if (static_cast<Constant*>(val)->getNodeType() == NodeType::constant) {
            is_propagational_ = true;
        }
        childs_.push_back(nullptr);
        childs_.push_back(nullptr);
    }

    void setPropagational(bool f) { is_propagational_ = f; }

    bool getPropagational() { return is_propagational_; }

    ~Expression() override {
        for (auto& c : childs_) {
            delete c;
        }
        childs_.clear();
    }

    void deleteChildren() {
        for (auto& c : childs_) {
            delete c;
        }
        childs_.clear();
        childs_.push_back(nullptr);
        childs_.push_back(nullptr);
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
    bool is_propagational_{false};
    ExprUnit expr_unit_;
    Node* value_{nullptr};
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_EXPRESSION_H_
