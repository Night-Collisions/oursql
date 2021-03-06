#include "Resolver.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <iostream>

#include "../../../Engine/IndexesManager.h"
#include "../Nodes/Ident.h"
#include "../Nodes/IntConstant.h"
#include "../Nodes/RealConstant.h"

using namespace boost::posix_time;

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
    // TODO: add all operations for datetime
    table1_ = table1;
    table2_ = table2;
    column_infos_ = column_infos;

    if (!e && root && root->getConstant() &&
        root->getConstant()->getNodeType() == NodeType::ident) {
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
    if (type1 == DataType::datetime && type2 == DataType::datetime) {
        res = std::to_string(value1 == value2);
    } else if (type1 != DataType::varchar && type2 != DataType::varchar) {
        res = std::to_string(std::stof(value1) == std::stof(value2));
    } else {
        res = std::to_string(value1 == value2);
    }
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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

    DataType type1 = DataType::Count;
    DataType type2 = DataType::Count;
    setDataTypes(root->childs()[0]->getConstant(),
                 root->childs()[1]->getConstant(), type1, type2, table1_,
                 table2_, column_infos_, e);

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
        if (type1 == DataType::datetime) {
            a = std::to_string(to_time_t(
                time_from_string(record[id->getTableName()][id->getName()])));
        } else {
            a = record[id->getTableName()][id->getName()];
        }
    } else {
        std::string val = static_cast<Constant*>(constant1)->getValue();
        if (type1 == DataType::datetime) {
            val = std::to_string(to_time_t(time_from_string(val)));
        } else if (type1 != DataType::varchar) {
            val = std::to_string(std::stof(val));
        }
        a = val;
    }

    if (constant2->getNodeType() == NodeType::ident) {
        auto id = static_cast<Ident*>(constant2);
        if (type2 == DataType::datetime) {
            b = std::to_string(to_time_t(
                time_from_string(record[id->getTableName()][id->getName()])));
        } else {
            b = record[id->getTableName()][id->getName()];
        }
    } else {
        std::string val = static_cast<Constant*>(constant2)->getValue();
        if (type2 == DataType::datetime) {
            val = std::to_string(to_time_t(time_from_string(val)));
        } else if (type2 != DataType::varchar) {
            val = std::to_string(std::stof(val));
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    bindColumnToTable(root->childs()[1]->getConstant(), e);

    if (child2->getConstant()->getNodeType() == NodeType::ident) {
        auto id = static_cast<Ident*>(child2->getConstant());
        value = record[id->getTableName()][id->getName()];
    } else {
        value = static_cast<Constant*>(child2->getConstant())->getValue();
    }

    DataType type2 = DataType::Count;
    setDataType(root->childs()[1]->getConstant(), type2, table2_,
                column_infos_[table2_], e);
    if (e) {
        return;
    }
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    if (root->childs()[0]->getPropagational() &&
        root->childs()[1]->getPropagational()) {
        root->deleteChildren();
        root->setPropagational(true);
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
    std::string table_name1 = table1;
    std::string table_name2 = table2;
    if (left->getNodeType() == NodeType::ident &&
        right->getNodeType() == NodeType::ident) {
        auto name1 = static_cast<Ident*>(left)->getTableName();
        auto name2 = static_cast<Ident*>(right)->getTableName();
        if (table1 == name2 && table2 == name1) {
            std::swap(table_name1, table_name2);
        }
    }

    if (left->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(left)->getName();
        if (column_infos[table_name1].find(col_name) !=
            column_infos[table_name1].end()) {
            a = column_infos[table_name1][col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table_name1));
            return;
        }
    } else {
        a = static_cast<Constant*>(left)->getDataType();
    }

    if (right->getNodeType() == NodeType::ident) {
        auto col_name = static_cast<Ident*>(right)->getName();
        if (column_infos[table_name2].find(col_name) !=
            column_infos[table_name2].end()) {
            b = column_infos[table_name2][col_name].getType();
        } else {
            e.reset(new exc::acc::ColumnNonexistent(col_name, table_name2));
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

    bool flag = false;
    if (column_infos_[table1_].find(id->getName()) !=
        column_infos_[table1_].end()) {
        id->setTableName(table1_);
        flag = true;
    }

    if (table1_ != table2_) {
        if (column_infos_[table2_].find(id->getName()) !=
            column_infos_[table2_].end()) {
            if (flag) {
                e.reset(new exc::AmbiguousColumnName("ambiguous column name " +
                                                     id->getName()));
                return;
            } else {
                id->setTableName(table2_);
            }
        }
    }
}

std::map<std::string, std::string> Resolver::getRecordMap(
    const std::vector<Column>& cols, std::vector<Value> record,
    std::unique_ptr<exc::Exception>& e) {
    std::map<std::string, std::string> m;
    int counter = 0;
    for (auto& k : cols) {
        auto colname = k.getName();
        if (record[counter].is_null) {
            if (k.getType() == DataType::varchar) {
                m[colname] = "";
            } else {
                m[colname] = "null";
            }
        } else {
            if (k.getType() == DataType::datetime) {
                unsigned long long t = std::stoull(record[counter].data);
                auto str_date = to_simple_string(from_time_t(t));
                m[colname] = str_date;
            } else {
                m[colname] = record[counter].data;
            }
        }

        ++counter;
    }
    return m;
}

bool Resolver::isGoodForIndex(const std::string& table_name, Expression* root,
                              std::map<std::string, int>& col_ind) {
    if (root && root->childs()[0] && root->childs()[1]) {
        auto child1 = root->childs()[0];
        auto child2 = root->childs()[1];

        bool is_ok1 = false;
        bool is_ok2 = false;
        bool index_found = false;
        bool oper_ok = false;

        auto type = static_cast<int>(root->exprType());
        auto left = static_cast<int>(ExprUnit::equal);
        auto right = static_cast<int>(ExprUnit::less_eq);
        if (left <= type && type <= right) {
            oper_ok = true;
        }
        if (child1->exprType() != ExprUnit::value) {
        } else if (child1->getConstant()->getNodeType() == NodeType::ident) {
            if (col_ind.find(child1->getConstant()->getName()) == col_ind.end()) {
                // TODO: no column
                return false;
            }

            if (IndexesManager::exists(table_name,
                                       col_ind[child1->getConstant()->getName()])) {
                is_ok1 = true;
                index_found = true;
            }
        } else if (child1->getConstant()->getNodeType() == NodeType::constant) {
            is_ok1 = true;
        }
        if (child2->exprType() != ExprUnit::value) {
            return false;
        } else if (child2->getConstant()->getNodeType() == NodeType::ident) {
            if (col_ind.find(child2->getConstant()->getName()) == col_ind.end()) {
                // TODO: exception, no column
                return false;
            }

            if (IndexesManager::exists(table_name,
                                       col_ind[child2->getConstant()->getName()])) {
                is_ok2 = true;
                index_found = !index_found;
            }
        } else if (child2->getConstant()->getNodeType() == NodeType::constant) {
            is_ok2 = true;
        }

        return is_ok1 && is_ok2 && index_found && oper_ok;
    }
    return false;
}
