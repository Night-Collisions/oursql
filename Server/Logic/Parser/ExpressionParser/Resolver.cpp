#include "Resolver.h"
#include <iostream>
#include "../Nodes/Ident.h"
#include "../Nodes/IntConstant.h"
#include "../Nodes/RealConstant.h"

std::array<func, static_cast<unsigned int>(ExprUnit::Count)>
    Resolver::operations_ = {equal,     notEqual, greater, greaterEqual, less,
                             lessEqual, logicAnd, logicOr, logicNot,     mul,
                             div,       add,      sub};
std::string Resolver::table1_;
std::string Resolver::table2_;
t_column_infos Resolver::column_infos_;

std::string Resolver::resolve(const std::string& table1,
                              const std::string& table2,
                              t_column_infos column_infos, Expression* root,
                              t_record_infos record,
                              std::unique_ptr<exc::Exception>& e) {
    table1_ = table1;
    table2_ = table2;
    column_infos_ = column_infos;


    if (!e && root && root->getConstant() && root->getConstant()->getNodeType() == NodeType::ident) {
        auto id = static_cast<Ident*>(root->getConstant());
        return record[id->getTableName()][id->getName()];
    }
    calculate(root, record, e);
    return (root && !e)
               ? ((static_cast<Constant*>(root->getConstant())->getValue() ==
                       "null" ||
                   static_cast<Constant*>(root->getConstant())->getValue() ==
                       "0.0")
                      ? ("0")
                      : (static_cast<Constant*>(root->getConstant())
                             ->getValue()))
               : ("1");
}

void Resolver::calculate(Expression* root, t_record_infos record,
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

bool Resolver::compareTypes(const std::string& table1,
                            const std::string& table2,
                            t_column_infos& column_info, Node* left,
                            Node* right, std::unique_ptr<exc::Exception>& e,
                            const CompareCondition& cond,
                            const std::string& oper = "") {
    DataType left_type = DataType::Count;
    DataType right_type = DataType::Count;

    setDataTypes(left, right, left_type, right_type, table1, table2,
                 column_info, e);
    if (e) {
        return false;
    }

    if (left_type == DataType::real && right_type == DataType::integer) {
        return true;
    }

    if (cond != CompareCondition::assign) {
        if (left_type == DataType::integer && right_type == DataType::real) {
            return true;
        }
    }

    if (right_type == DataType::null_) {
        return true;
    }

    if (left_type == right_type) {
        return true;
    } else {
        if (cond == CompareCondition::compare) {
            e.reset(new exc::CompareDataTypeMismatch(left_type, right_type));
        }
        if (cond == CompareCondition::assign) {
            e.reset(new exc::SetDataTypeMismatch(
                left_type, static_cast<Ident*>(left)->getName()));
        }
        if (cond == CompareCondition::operation) {
            e.reset(new exc::NoOperationForType(left_type, oper, right_type));
        }
        return false;
    }
}

void Resolver::equal(Expression* root, t_record_infos record,
                     std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::compare,
                   "=");
    if (e) {
        return;
    }
    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
    if (e) {
        return;
    }

    std::string res;
    if (type1 != DataType::varchar && type2 != DataType::varchar) {
        res = std::to_string(std::stof(value1) == std::stof(value2));
    } else {
        res = std::to_string(value1 == value2);
    }
    root->setConstant(new IntConstant(res));
}

void Resolver::notEqual(Expression* root, t_record_infos record,
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

void Resolver::greater(Expression* root, t_record_infos record,
                       std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::compare,
                   ">");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            if (value1 == value2) {
                res = 1;
            } else {
                res = 0;
            }
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::setStringValue(Expression* root, t_record_infos record,
                              std::unique_ptr<exc::Exception>& e,
                              std::string& a, std::string& b,
                              const CompareCondition& cond,
                              const std::string& oper) {
    auto child1 = root->childs()[0];
    auto child2 = root->childs()[1];

    if (!compareTypes(table1_, table2_, column_infos_, child1->getConstant(),
                      child2->getConstant(), e, cond, oper)) {
        return;
    }

    auto constant1 = child1->getConstant();
    auto constant2 = child2->getConstant();

    bindColumnToTable(constant1, e);
    if (e) {
        return;
    }
    bindColumnToTable(constant2, e);
    if (e) {
        return;
    }

    if (constant1->getNodeType() == NodeType::ident) {
        auto id = static_cast<Ident*>(constant1);
        a = record[table1_][id->getName()];
    } else {
        std::string val = static_cast<Constant*>(constant1)->getValue();
        if (static_cast<Constant*>(constant1)->getDataType() !=
            DataType::varchar) {
            val = std::to_string(
                std::stof(static_cast<Constant*>(constant1)->getValue()));
        }
        a = val;
    }

    if (constant2->getNodeType() == NodeType::ident) {
        auto id = static_cast<Ident*>(constant2);
        b = record[table2_][id->getName()];
    } else {
        std::string val = static_cast<Constant*>(constant2)->getValue();
        if (static_cast<Constant*>(constant2)->getDataType() !=
            DataType::varchar) {
            val = std::to_string(
                std::stof(static_cast<Constant*>(constant2)->getValue()));
        }
        b = val;
    }
}

void Resolver::greaterEqual(Expression* root, t_record_infos record,
                            std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::compare,
                   ">=");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            if (value1 == value2) {
                res = 1;
            } else {
                res = 0;
            }
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::less(Expression* root, t_record_infos record,
                    std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::compare,
                   "<");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            if (value1 == value2) {
                res = 1;
            } else {
                res = 0;
            }
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::lessEqual(Expression* root, t_record_infos record,
                         std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::compare,
                   "<=");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            if (value1 == value2) {
                res = 1;
            } else {
                res = 0;
            }
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::logicAnd(Expression* root, t_record_infos record,
                        std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::operation,
                   "and");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            if (value1 == value2) {
                res = 1;
            } else {
                res = 0;
            }
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::logicOr(Expression* root, t_record_infos record,
                       std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::operation,
                   "or");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            if (value1 == value2) {
                res = 1;
            } else {
                res = 0;
            }
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::div(Expression* root, t_record_infos record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::operation,
                   "/");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            e.reset(new exc::NoOperationForType());
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }
}

void Resolver::logicNot(Expression* root, t_record_infos record,
                        std::unique_ptr<exc::Exception>& e) {
    auto child2 = root->childs()[1];
    std::string value;

    if (child2->getConstant()->getNodeType() == NodeType::ident) {
        auto id = static_cast<Ident*>(child2->getConstant());
        value = record[table2_][id->getName()];
    } else {
        value = static_cast<Constant*>(child2->getConstant())->getValue();
    }

    DataType type2 = DataType::Count;
    setDataType(root->childs()[1]->getConstant(), type2, table1_,
                column_infos_[table1_], e);
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
        if (value == "null") {
            e.reset(new exc::NoOperationForType());
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }

    root->setConstant(new IntConstant(std::to_string(res)));
}

void Resolver::mul(Expression* root, t_record_infos record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::operation,
                   "*");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
            e.reset(new exc::NoOperationForType(DataType::varchar, "+",
                                                DataType::varchar));
            return;
        }
    } catch (std::invalid_argument& tmp) {
        if (value1 == "null" || value2 == "null") {
            e.reset(new exc::NoOperationForType());
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }
}

void Resolver::add(Expression* root, t_record_infos record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::operation,
                   "+");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
            return;
        }
    } catch (std::invalid_argument& tmp) {
        if (value1 == "null" || value2 == "null") {
            e.reset(new exc::NoOperationForType());
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }
}

void Resolver::sub(Expression* root, t_record_infos record,
                   std::unique_ptr<exc::Exception>& e) {
    std::string value1;
    std::string value2;
    setStringValue(root, record, e, value1, value2, CompareCondition::operation,
                   "-");
    if (e) {
        return;
    }

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);
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
        if (value1 == "null" || value2 == "null") {
            e.reset(new exc::NoOperationForType());
        } else {
            e.reset(new exc::DataTypeOversize());
            return;
        }
    }
}

void Resolver::setDataTypes(Node* left, Node* right, DataType& a, DataType& b,
                            const std::string& table1,
                            const std::string& table2,
                            t_column_infos column_infos,
                            std::unique_ptr<exc::Exception>& e) {
    if (left->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(left)->getName();
        if (column_infos[table1].find(col_name) != column_infos[table1].end()) {
            a = column_infos[table1][col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table1));
            return;
        }
    } else {
        a = static_cast<Constant*>(left)->getDataType();
    }

    if (right->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(right)->getName();
        if (column_infos[table2].find(col_name) != column_infos[table2].end()) {
            b = column_infos[table2][col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table2));
            return;
        };
    } else {
        b = static_cast<Constant*>(right)->getDataType();
    }
}

void Resolver::setDataType(Node* nod, DataType& a, const std::string& table,
                           std::map<std::string, Column> column_info,
                           std::unique_ptr<exc::Exception>& e) {
    if (nod->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(nod)->getName();
        if (column_info.find(col_name) != column_info.end()) {
            a = column_info[col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table));
            return;
        };
    } else {
        a = static_cast<Constant*>(nod)->getDataType();
    }
}

void Resolver::bindColumnToTable(Node* nod,
                                 std::unique_ptr<exc::Exception>& e) {
    if (nod->getNodeType() != NodeType::ident) {
        return;
    }

    auto id = static_cast<Ident*>(nod);
    if (!id->getTableName().empty()) {
        return;
    }

    if (column_infos_[table1_].find(id->getName()) !=
        column_infos_[table1_].end()) {
        id->setTableName(table1_);
    }

    if (table1_ != table2_) {
        if (column_infos_[table2_].find(id->getName()) !=
            column_infos_[table2_].end()) {
            if (!id->getTableName().empty()) {
                e.reset(new exc::AmbiguousColumnName("ambiguous column name " +
                                                     id->getName()));
                return;
            } else {
                id->setTableName(table2_);
            }
        }
    }
}

std::map<std::string, std::string> Resolver::getRecord(
    const std::vector<Column>& cols, std::vector<Value> record) {
    std::map<std::string, std::string> m;
    int counter = 0;
    for (auto& k : cols) {
        auto c = k.getName();
        if (record[counter].is_null) {
            if (k.getType() == DataType::varchar) {
                m[c] = "";
            } else {
                m[c] = "null";
            }
        } else {
            m[c] = record[counter].data;
        }

        ++counter;
    }
    return m;
}