#include "Resolver.h"
#include "../Nodes/Ident.h"
#include "../Nodes/IntConstant.h"

void Resolver::resolve(std::string table,
                       std::map<std::string, Column> all_columns,
                       Expression* root, const rapidjson::Value& record,
                       std::unique_ptr<exc::Exception>& e) {
    void (*const operations[static_cast<unsigned int>(ExprUnit::Count)])(
        Expression * root, const rapidjson::Value& record,
        std::unique_ptr<exc::Exception>& e) = {
        [](Expression* root, const rapidjson::Value& record,
           std::unique_ptr<exc::Exception>& e) { assert(false); },
        equal,
        notEqual,
        greater,
        greaterEqual,
        less,
        lessEqual,
        logicAnd,
        logicOr,
        logicNot,
        mul,
        div,
        add,
        sub,
        deductVal};

    operations_ = operations;  // ДАНИЛА, НЕ РАБОТАЕТ

    table_ = table;
    all_columns_ = all_columns;
}

void Resolver::calculate(Expression* root, const rapidjson::Value& record,
                         std::unique_ptr<exc::Exception>& e) {
    if (e) {
        return;
    }

    if (root && root->childs()[0] && root->childs()[1]) {
        auto child1 = root->childs()[0];
        auto child2 = root->childs()[1];

        calculate(child1, record, e);
        if (e) {
            return;
        }
        calculate(child2, record, e);
        if (e) {
            return;
        }

        operations_[static_cast<unsigned int>(root->exprType())](root, e);
        if (e) {
            return;
        }
    }
}

bool Resolver::compareTypes(const std::string& table_name,
                            std::map<std::string, Column>& all_columns,
                            Node* left, Node* right,
                            std::unique_ptr<exc::Exception>& e, bool is_set) {
    DataType left_type = DataType::Count;
    DataType right_type = DataType::Count;

    if (left->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(left)->getName();
        if (all_columns.find(col_name) != all_columns.end()) {
            left_type = all_columns[col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table_name));
            return false;
        };
    } else {
        left_type = static_cast<Constant*>(left)->getDataType();
    }

    if (right->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(right)->getName();
        if (all_columns.find(col_name) != all_columns.end()) {
            right_type = all_columns[col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table_name));
            return false;
        };
    } else {
        right_type = static_cast<Constant*>(right)->getDataType();
    }

    if (left_type == DataType::Count) {
        e.reset(new exc::acc::ColumnNonexistent(
            static_cast<Ident*>(left)->getName(), table_name));
        return false;
    }

    if (right_type == DataType::Count) {
        e.reset(new exc::acc::ColumnNonexistent(
            static_cast<Ident*>(right)->getName(), table_name));
        return false;
    }

    if (left_type == DataType::real && right_type == DataType::integer) {
        return true;
    }

    if (right_type == DataType::null_) {
        return true;
    }

    if (left_type == right_type) {
        return true;
    } else {
        if (!is_set) {
            e.reset(new exc::CompareDataTypeMismatch(left_type, right_type));
        } else {
            e.reset(new exc::SetDataTypeMismatch(
                left_type, static_cast<Ident*>(left)->getName()));
        }
        return false;
    }
}

void Resolver::equal(Expression* root, const rapidjson::Value& record,
                     std::unique_ptr<exc::Exception>& e) {
    auto child1 = root->childs()[0];
    auto child2 = root->childs()[1];

    if (child1->exprType() != ExprUnit::value ||
        child2->exprType() != ExprUnit::value) {
        e.reset();  // TODO
        return;
    }

    if (!compareTypes(table_, all_columns_, child1, child2, e, false)) {
        return;
    }

    std::string value1;
    std::string value2;

    if (child1->getNodeType() == NodeType::ident) {
        value1 = record[child1->getName()].GetString();
    } else {
        value1 = static_cast<Constant*>(child1->getConstant())->getValue();
    }

    if (child2->getNodeType() == NodeType::ident) {
        value2 = record[child2->getName()].GetString();
    } else {
        value2 = static_cast<Constant*>(child2->getConstant())->getValue();
    }

    std::string res = std::to_string(value1 == value2);
    root->setVal(new IntConstant(res));
    // root->setExprType(ExprUnit::value);
}
