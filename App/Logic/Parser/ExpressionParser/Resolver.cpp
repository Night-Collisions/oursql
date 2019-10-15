#include "Resolver.h"
#include "../Nodes/Ident.h"

void Resolver::resolve(Expression* root, const rapidjson::Value& record) {

}

bool Resolver::compareTypes(const std::string& table_name,
                                std::map<std::string, Column>& all_columns,
                                Node* left, Node* right,
                                std::unique_ptr<exc::Exception>& e,
                                bool is_set) {
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