#include "Resolver.h"
#include "../Nodes/Ident.h"
#include "../Nodes/IntConstant.h"
#include "../Nodes/RealConstant.h"

std::array<func, static_cast<unsigned int>(ExprUnit::Count)>
    Resolver::operations_ = {equal,     notEqual, greater, greaterEqual, less,
                             lessEqual, logicAnd, logicOr, logicNot,     mul,
                             div,       add,      sub};
std::string Resolver::table_;
std::map<std::string, Column> Resolver::all_columns_;

void Resolver::resolve(const std::string& table,
                       std::map<std::string, Column> all_columns,
                       Expression* root, const rapidjson::Value& record,
                       std::unique_ptr<exc::Exception>& e) {
    table_ = table;
    all_columns_ = std::move(all_columns);

    calculate(root, record, e);
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

        operations_[static_cast<unsigned int>(root->exprType())](root, record,
                                                                 e);
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

    setDataTypes(left, right, left_type, right_type, table_name, all_columns,
                 e);
    if (e) {
        return false;
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

    if (!is_set) {
        if (right_type == DataType::real && left_type == DataType::integer) {
            return true;
        }
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
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    std::string res = std::to_string(value1 == value2);
    root->setConstant(new IntConstant(res));
}

void Resolver::notEqual(Expression* root, const rapidjson::Value& record,
                        std::unique_ptr<exc::Exception>& e) {
    equal(root, record, e);
    if (e) {
        return;
    }
    auto res = static_cast<Constant*>(root->getConstant())->getValue();
    try {
        root->setConstant(new IntConstant(std::to_string(!std::stoi(res))));
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }
}

void Resolver::greater(Expression* root, const rapidjson::Value& record,
                       std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    int res = 0;
    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            res = a > b;
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            res = a > b;
        } else {
            res = value1 > value2;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::setStringValue(Expression* root, const rapidjson::Value& record,
                              std::unique_ptr<exc::Exception>& e,
                              std::string& a, std::string& b) {
    auto child1 = root->childs()[0];
    auto child2 = root->childs()[1];

    if (!compareTypes(table_, all_columns_, child1->getConstant(),
                      child2->getConstant(), e, false)) {
        return;
    }

    if (child1->getConstant()->getNodeType() == NodeType::ident) {
        a = record[child1->getConstant()->getName()].GetString();
    } else {
        a = static_cast<Constant*>(child1->getConstant())->getValue();
    }

    if (child2->getConstant()->getNodeType() == NodeType::ident) {
        b = record[child2->getConstant()->getName()].GetString();
    } else {
        b = static_cast<Constant*>(child2->getConstant())->getValue();
    }
}

void Resolver::greaterEqual(Expression* root, const rapidjson::Value& record,
                            std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    int res = 0;
    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            res = a >= b;
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            res = a >= b;
        } else {
            res = value1 >= value2;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::less(Expression* root, const rapidjson::Value& record,
                    std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    int res = 0;
    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            res = a < b;
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            res = a < b;
        } else {
            res = value1 < value2;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::lessEqual(Expression* root, const rapidjson::Value& record,
                         std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    int res = 0;
    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            res = a <= b;
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            res = a <= b;
        } else {
            res = value1 <= value2;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::logicAnd(Expression* root, const rapidjson::Value& record,
                        std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    int res = 0;
    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            res = a && b;
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            res = a && b;
        } else {
            e.reset(new exc::NoOperationForType(DataType::varchar, "and",
                                                DataType::varchar));
            return;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::logicOr(Expression* root, const rapidjson::Value& record,
                       std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    int res = 0;
    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            res = a || b;
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            res = a || b;
        } else {
            e.reset(new exc::NoOperationForType(DataType::varchar, "or",
                                                DataType::varchar));
            return;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::div(Expression* root, const rapidjson::Value& record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            if (b == 0) {
                e.reset(new exc::DivByZero(value1 + "/" + value2));
                return;
            }

            root->setConstant(new IntConstant(std::to_string(a / b)));
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);
            if (b == 0) {
                e.reset(new exc::DivByZero(value1 + "/" + value2));
                return;
            }

            root->setConstant(new RealConstant(std::to_string(a / b)));
        } else {
            e.reset(new exc::NoOperationForType(DataType::varchar, "/",
                                                DataType::varchar));
            return;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }
}

void Resolver::logicNot(Expression* root, const rapidjson::Value& record,
                        std::unique_ptr<exc::Exception>& e) {
    auto child2 = root->childs()[1];
    std::string value;

    if (child2->getConstant()->getNodeType() == NodeType::ident) {
        value = record[child2->getName()].GetString();
    } else {
        value = static_cast<Constant*>(child2->getConstant())->getValue();
    }

    DataType type2 = DataType::Count;
    setDataType(root->childs()[1]->getConstant(), type2, table_, all_columns_,
                e);
    if (e) {
        return;
    }

    int res = 0;
    try {
        if (type2 == DataType::integer) {
            auto a = std::stoi(value);

            res = !a;
        } else if (type2 == DataType::real) {
            auto a = std::stof(value);

            res = !a;
        } else {
            e.reset(new exc::NoOperationForType("not", DataType::varchar));
            return;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::mul(Expression* root, const rapidjson::Value& record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            root->setConstant(new IntConstant(std::to_string(a * b)));
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            root->setConstant(new RealConstant(std::to_string(a * b)));
        } else {
            e.reset(new exc::NoOperationForType(DataType::varchar, "-",
                                                DataType::varchar));
            return;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }
}

void Resolver::add(Expression* root, const rapidjson::Value& record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            root->setConstant(new IntConstant(std::to_string(a + b)));
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            root->setConstant(new RealConstant(std::to_string(a + b)));
        } else {
            e.reset(new exc::NoOperationForType(DataType::varchar, "+",
                                                DataType::varchar));
            return;  // todo
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }
}

void Resolver::sub(Expression* root, const rapidjson::Value& record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2);
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table_,
                 all_columns_, e);
    if (e) {
        return;
    }

    try {
        if (type1 == DataType::integer && type2 == DataType::integer) {
            auto a = std::stoi(value1);
            auto b = std::stoi(value2);

            root->setConstant(new IntConstant(std::to_string(a - b)));
        } else if (type1 == DataType::real || type2 == DataType::real) {
            auto a = std::stof(value1);
            auto b = std::stof(value2);

            root->setConstant(new RealConstant(std::to_string(a - b)));
        } else {
            e.reset(new exc::NoOperationForType(DataType::varchar, "-",
                                                DataType::varchar));
            return;
        }
    } catch (std::invalid_argument& tmp) {
        e.reset(new exc::DataTypeOversize());
        return;
    }
}

void Resolver::setDataTypes(Node* left, Node* right, DataType& a, DataType& b,
                            const std::string& table,
                            std::map<std::string, Column> all_columns,
                            std::unique_ptr<exc::Exception>& e) {
    if (left->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(left)->getName();
        if (all_columns.find(col_name) != all_columns.end()) {
            a = all_columns[col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table));
            return;
        };
    } else {
        a = static_cast<Constant*>(left)->getDataType();
    }

    if (right->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(right)->getName();
        if (all_columns.find(col_name) != all_columns.end()) {
            b = all_columns[col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table));
            return;
        };
    } else {
        b = static_cast<Constant*>(right)->getDataType();
    }
}

void Resolver::setDataType(Node* nod, DataType& a, const std::string& table,
                           std::map<std::string, Column> all_columns,
                           std::unique_ptr<exc::Exception>& e) {
    if (nod->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(nod)->getName();
        if (all_columns.find(col_name) != all_columns.end()) {
            a = all_columns[col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table));
            return;
        };
    } else {
        a = static_cast<Constant*>(nod)->getDataType();
    }
}
