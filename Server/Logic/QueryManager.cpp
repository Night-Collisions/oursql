#include "QueryManager.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <iostream>
#include <memory>
#include <mutex>

#include "../Engine/Column.h"
#include "../Engine/Cursor.h"
#include "../Engine/Engine.h"
#include "Parser/ExpressionParser/Resolver.h"
#include "Parser/Nodes/ConstantList.h"
#include "Parser/Nodes/DatetimeConstant.h"
#include "Parser/Nodes/Ident.h"
#include "Parser/Nodes/IdentList.h"
#include "Parser/Nodes/IndexNode.h"
#include "Parser/Nodes/IntConstant.h"
#include "Parser/Nodes/Period.h"
#include "Parser/Nodes/RelExpr.h"
#include "Parser/Nodes/SelectList.h"
#include "Parser/Nodes/SysTime.h"
#include "Parser/Nodes/TextConstant.h"
#include "Parser/Nodes/VarList.h"
#include "Parser/Nodes/With.h"
#include "Parser/RelationalOperationsParser/Helper.h"
#include "Parser/RelationalOperationsParser/Intersect.h"
#include "Parser/RelationalOperationsParser/Join.h"
#include "Parser/RelationalOperationsParser/Union.h"

std::mutex transact_mtx;
using namespace boost::gregorian;
using namespace boost::posix_time;

std::string getPtimeToPosixStr(const ptime& t) {
    return std::to_string(to_time_t(t));
}

ptime getPosixStrToPtime(const std::string& s) {
    unsigned long long t = std::stoull(s);
    return from_time_t(t);
}

void QueryManager::execute(
    const Query& query, t_ull transact_num, std::unique_ptr<exc::Exception>& e,
    std::ostream& out,
    std::map<unsigned long long, std::set<std::string>>& locked_tables) {
    void (*const
              commandsActions[static_cast<unsigned int>(CommandType::Count)])(
        const Query& query, t_ull transact_num,
        std::unique_ptr<exc::Exception>& e, std::ostream& out) = {
        [](const Query&, t_ull transact_num, std::unique_ptr<exc::Exception>& e,
           std::ostream& out) { assert(false); },
        createTable,
        showCreateTable,
        dropTable,
        select,
        insert,
        update,
        remove,
        createIndex};
    CommandType command = query.getCmdType();
    if (command != CommandType::Count) {
        if (command != CommandType::create_table &&
            command != CommandType::select &&
            command != CommandType::create_index) {
            auto name = query.getChildren()[NodeType::ident]->getName();
            if (!Engine::exists(name)) {
                e.reset(new exc::acc::TableNonexistent(name));
                return;
            }
            auto table = Engine::show(name);

            {
                std::unique_lock<std::mutex> table_lock(transact_mtx);
                for (auto& s : locked_tables) {
                    if (s.second.find(table.getName()) != s.second.end()) {
                        e.reset(new exc::tr::SerializeAccessError());
                        Engine::endTransaction(transact_num);
                        return;
                    }
                }
            }
            commandsActions[static_cast<unsigned int>(command)](
                query, transact_num, e, out);

        } else {
            commandsActions[static_cast<unsigned int>(command)](
                query, transact_num, e, out);
        }
    }
}

void QueryManager::createTemporalTable(const std::string& name,
                                       const std::vector<Column>& columns,
                                       std::unique_ptr<exc::Exception>& e) {
    std::vector<Column> temp_columns(columns.size());
    std::copy(columns.begin(), columns.end(), temp_columns.begin());

    for (auto& c : temp_columns) {
        c.setConstraints({});
    }

    Table table(name, temp_columns, e);
    if (e) {
        return;
    }
    Engine::create(table, e);
}

void QueryManager::createTable(const Query& query, t_ull transact_num,
                               std::unique_ptr<exc::Exception>& e,
                               std::ostream& out) {
    e.reset(nullptr);
    std::string name = query.getChildren()[NodeType::ident]->getName();

    auto vars = static_cast<VarList*>(query.getChildren()[NodeType::var_list])
                    ->getVars();
    auto period =
        static_cast<Period*>(query.getChildren()[NodeType::period_pair])
            ->getPeriod();
    ;
    bool is_period = false;
    is_period = !(period.first.empty() || period.second.empty());

    std::vector<Column> columns;
    bool sys_start_found = false;
    bool sys_end_found = false;
    bool has_primary_key = false;
    for (auto& v : vars) {
        std::string col_name = v->getName();
        DataType type = v->getType();
        int len = 0;
        if (type == DataType::varchar) {
            len = v->getVarcharLen();
        }

        std::set<ColumnConstraint> constr_set;

        for (auto& c : v->getConstraints()) {
            if (constr_set.find(c) == constr_set.end()) {
                if (c == ColumnConstraint::primary_key) {
                    has_primary_key = true;
                }
                constr_set.insert(c);
            } else {
                e.reset(
                    new exc::constr::RedundantConstraints(name, v->getName()));
                return;
            }
        }

        Column col(col_name, type, e, constr_set);
        col.setN(len);
        if (period.first == col_name) {
            if (type != DataType::datetime) {
                e.reset(new exc::IncorrectTypeForPeriod());
                return;
            }
            col.setPeriod(PeriodState::sys_start);
            sys_start_found = true;
        }
        if (period.second == col_name) {
            if (type != DataType::datetime) {
                e.reset(new exc::IncorrectTypeForPeriod());
                return;
            }
            col.setPeriod(PeriodState::sys_end);
            sys_end_found = true;
        }
        columns.emplace_back(col);
    }
    bool is_period_correct = sys_start_found && sys_end_found;
    if (!is_period_correct && is_period) {
        e.reset(new exc::acc::ColumnNonexistent());
        return;
    }

    if (is_period && !has_primary_key) {
        e.reset(new exc::cr_table::PrimaryKeyInTempTable());
        return;
    }

    if (is_period) {
        createTemporalTable(getHistoryName(name), columns, e);
        if (e) {
            return;
        }
    }

    Table table(name, columns, e, is_period_correct);
    if (e != nullptr) {
        return;
    }

    Engine::create(table, e);
}

void QueryManager::showCreateTable(const Query& query, t_ull transact_num,
                                   std::unique_ptr<exc::Exception>& e,
                                   std::ostream& out) {
    auto name = query.getChildren()[NodeType::ident]->getName();
    if (!Engine::exists(name)) {
        e.reset(new exc::acc::TableNonexistent(name));
        return;
    }
    auto res = Engine::showCreate(name, e);
    out << res << std::endl;
}

void QueryManager::dropTable(const Query& query, t_ull transact_num,
                             std::unique_ptr<exc::Exception>& e,
                             std::ostream& out) {
    auto name = query.getChildren()[NodeType::ident]->getName();
    if (Engine::show(name).isSystemVersioning()) {
        e.reset(new exc::TemporalTableDropNotAllowed());
        return;
    } else {
        Engine::drop(name, e);
    }
}

void printSelectedRecords(const Table& table, t_column_infos column_infos,
                          std::vector<Node*> cols_from_parser,
                          const std::vector<Value>& record, Expression* root,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    t_record_infos record_infos;
    record_infos[table.getName()] =
        Resolver::getRecordMap(table.getColumns(), record, e);
    if (e) {
        return;
    }

    std::string response = Resolver::resolve(
        table.getName(), table.getName(), column_infos, root, record_infos, e);
    if (e) {
        return;
    }
    if (response != "0") {
        std::string response;
        int expr_cnt = 1;
        out << "=======\n";
        for (auto& c : cols_from_parser) {
            auto expr = static_cast<Expression*>(c);
            std::string prefix = Helper::getCorrectTablePrefix(table.getName());
            if (expr->exprType() == ExprUnit::value &&
                expr->getConstant()->getName() == "*") {
                for (auto& k : table.getColumns()) {
                    out << prefix + k.getName() + ": " +
                               record_infos[table.getName()][k.getName()]
                        << std::endl;
                }
                continue;
            }
            response = Resolver::resolve(table.getName(), table.getName(),
                                         column_infos, expr, record_infos, e);
            std::string colname = expr->getConstant()->getName();
            if (colname.empty()) {
                colname = "expression " + std::to_string(expr_cnt++);
            } else {
                auto id = static_cast<Ident*>(expr->getConstant());
                colname = (id->getTableName().empty())
                              ? ("")
                              : (id->getTableName() + ".");
                colname += id->getName();
            }
            if (e) {
                return;
            }
            out << colname + ": " + response << std::endl;
        }
        if (e) {
            return;
        }
    }
}

std::map<std::string, Column> getColumnMap(const Table& t) {
    std::map<std::string, Column> all_columns;
    for (auto& c : t.getColumns()) {
        all_columns[c.getName()] = c;
    }

    return all_columns;
}

std::map<std::string, int> getColumnIndexMap(const Table& t) {
    std::map<std::string, int> all_columns;
    auto cols = t.getColumns();
    for (int i = 0; i < cols.size(); ++i) {
        all_columns[cols[i].getName()] = i;
    }

    return all_columns;
}

void QueryManager::select(const Query& query, t_ull transact_num,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    auto children = query.getChildren();
    t_column_infos column_info;
    std::vector<Node*> cols_from_parser;
    Table resolvedTable;

    bool in_memory = false;

    if (children.find(NodeType::relational_oper_expr) != children.end()) {
        auto root =
            static_cast<RelExpr*>(children[NodeType::relational_oper_expr]);
        resolvedTable = resolveRelationalOperTree(root, transact_num, e);
        in_memory = true;
        if (e) {
            return;
        }

    } else if (children.find(NodeType::sys_time) != children.end()) {
        auto name = children[NodeType::ident]->getName();
        auto systime = *static_cast<SysTime*>(children[NodeType::sys_time]);
        in_memory = true;
        if (!Engine::exists(name)) {
            e.reset(new exc::acc::TableNonexistent(name));
            return;
        }
        resolvedTable = getFilledTempTable(name, transact_num, systime, e);
        if (e) {
            return;
        }
        name = resolvedTable.getName();
        int sys_start_ind = 0;
        int sys_end_ind = 0;
        auto columns = resolvedTable.getColumns();
        for (int i = 0; i < columns.size(); ++i) {
            if (columns[i].getPeriod() == PeriodState::sys_start) {
                sys_start_ind = i;
            } else if (columns[i].getPeriod() == PeriodState::sys_end) {
                sys_end_ind = i;
            }
        }
        if (systime.getRangeType() == RangeType::from_to) {
            auto [from, to] = systime.getRange();
            auto fromNode = new Expression(new DatetimeConstant(from));
            auto toNode = new Expression(new DatetimeConstant(to));
            auto start_col =
                new Expression(new Ident(columns[sys_start_ind].getName()));
            auto end_col =
                new Expression(new Ident(columns[sys_end_ind].getName()));
            auto cond1 = new Expression(fromNode, ExprUnit::less_eq, start_col);
            auto cond2 = new Expression(end_col, ExprUnit::less_eq, toNode);
            auto rangeCond = new Expression(cond1, ExprUnit::and_, cond2);
            if (children.find(NodeType::expression) != children.end()) {
                auto prev =
                    static_cast<Expression*>(children[NodeType::expression]);
                auto new_expr = new Expression(prev, ExprUnit::and_, rangeCond);
                children[NodeType::expression] = new_expr;
            } else {
                children[NodeType::expression] = rangeCond;
            }
        }
    } else {
        auto name = children[NodeType::ident]->getName();
        if (!Engine::exists(name)) {
            e.reset(new exc::acc::TableNonexistent(name));
            return;
        }
        resolvedTable = Engine::show(name);

        if (e) {
            return;
        }
    }

    column_info[resolvedTable.getName()] = getColumnMap(resolvedTable);

    auto column_index = getColumnIndexMap(resolvedTable);

    cols_from_parser =
        static_cast<SelectList*>(children[NodeType::select_list])->getList();

    for (auto& c : cols_from_parser) {
        std::string colname;
        std::string tablename;
        auto expr = static_cast<Expression*>(c);
        if (expr->exprType() == ExprUnit::value &&
            expr->getConstant()->getName() == "*") {
            continue;
        }
        auto node = expr->getConstant();
        if (node == nullptr) {
            continue;
        }
        if (node->getNodeType() == NodeType::ident) {
            auto id = static_cast<Ident*>(node);
            tablename = resolvedTable.getName();
            colname = id->getName();
            if (tablename.empty()) {
                colname = (id->getTableName().empty())
                              ? (id->getName())
                              : (id->getTableName() + "." + id->getName());
            }

            id->setTableName(tablename);
            id->setName(colname);
        } else {
            colname = node->getName();
        }

        if (node->getNodeType() == NodeType::ident &&
            column_info[resolvedTable.getName()].find(colname) ==
                column_info[resolvedTable.getName()].end()) {
            e.reset(new exc::acc::ColumnNonexistent(colname,
                                                    resolvedTable.getName()));
            return;
        }
    }

    auto root = static_cast<Expression*>(children[NodeType::expression]);

    bool use_index =
        Resolver::isGoodForIndex(resolvedTable.getName(), root, column_index);

    if (in_memory) {
        auto records = resolvedTable.getRecords();
        for (int i = 0; i < records.size(); ++i) {
            printSelectedRecords(resolvedTable, column_info, cols_from_parser,
                                 records[i], root, e, out);
            if (e) {
                return;
            }
        }
    } else if (!use_index) {
        Cursor cursor(transact_num, resolvedTable.getName());
        while (cursor.next()) {
            auto ftch = cursor.fetch();
            printSelectedRecords(resolvedTable, column_info, cols_from_parser,
                                 ftch, root, e, out);
            if (e) {
                return;
            }
        }
    } else {
        Cursor cursor(transact_num, resolvedTable.getName());
        auto adress_iter = getRecordsFromIndex(resolvedTable.getName(), root);
        for (auto& i = adress_iter.first; i != adress_iter.second; ++i) {
            cursor.setPosition(i->second);
            auto record = cursor.fetch();
            printSelectedRecords(resolvedTable, column_info, cols_from_parser,
                                 record, root, e, out);
            if (e) {
                return;
            }
        }
    }
}

void QueryManager::insert(const Query& query, t_ull transact_num,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    e.reset(nullptr);

    auto name = query.getChildren()[NodeType::ident]->getName();
    auto table = Engine::show(name);

    std::map<std::string, std::map<std::string, Column>> column_info;

    for (auto& c : table.getColumns()) {
        if (c.getPeriod() == PeriodState::none) {
            column_info[table.getName()][c.getName()] = c;
        }
    }

    auto idents =
        static_cast<IdentList*>(query.getChildren()[NodeType::ident_list])
            ->getIdents();
    std::set<std::string> col_set;
    if (!idents.empty()) {
        for (auto& c : idents) {
            if (col_set.find(c->getName()) == col_set.end()) {
                col_set.insert(c->getName());
            } else {
                e.reset(new exc::RepeatColumn(c->getName()));
                return;
            }
        }
    } else {
        for (auto& c : table.getColumns()) {
            if (c.getPeriod() == PeriodState::none) {
                col_set.insert(c.getName());
            }
            idents.push_back(new Ident(c.getName()));
        }
    }

    auto constants =
        static_cast<ConstantList*>(query.getChildren()[NodeType::constant_list])
            ->getConstants();

    if (constants.size() > idents.size()) {
        e.reset(new exc::ins::ConstantsMoreColumns());
        return;
    }

    std::map<std::string, std::string> values;

    for (auto& c : table.getColumns()) {
        if ((col_set.find(c.getName()) == col_set.end() &&
             c.getConstraints().find(ColumnConstraint::not_null) !=
                 c.getConstraints().end()) ||
            (col_set.find(c.getName()) == col_set.end() &&
             c.getConstraints().find(ColumnConstraint::primary_key) !=
                 c.getConstraints().end())) {
            e.reset(new exc::constr::NullNotNull(name, c.getName()));
            return;
        }
    }

    for (size_t i = 0; i < constants.size(); ++i) {
        if (Resolver::compareTypes(name, name, column_info, idents[i],
                                   constants[i], e, CompareCondition::assign,
                                   "=")) {
            if (column_info[name][idents[i]->getName()].getType() ==
                    DataType::varchar &&
                column_info[name][idents[i]->getName()].getN() <
                    static_cast<Constant*>(constants[i])->getValue().length()) {
                e.reset(new exc::DataTypeOversize(idents[i]->getName()));
                return;
            }
            if (static_cast<Constant*>(constants[i])->getDataType() !=
                DataType::null_) {
                auto val = static_cast<Constant*>(constants[i])->getValue();
                if (column_info[name][idents[i]->getName()].getType() ==
                    DataType::real) {
                    values[idents[i]->getName()] =
                        std::to_string(std::stof(val));
                } else {
                    values[idents[i]->getName()] = std::string(val.c_str());
                }
            }
        } else {
            return;
        }
    }

    Cursor cursor(transact_num, name);

    std::vector<Value> v_arr;
    for (auto& c : table.getColumns()) {
        if (c.getPeriod() == PeriodState::sys_start) {
            Value v;
            v.is_null = false;
            auto curr = boost::posix_time::second_clock::local_time();
            v.data = getPtimeToPosixStr(curr);
            v_arr.push_back(v);
            continue;
        }
        if (c.getPeriod() == PeriodState::sys_end) {
            Value v;
            v.is_null = false;
            // TODO(Victor): 2038 issue
            //  boost got a 2038 issue but it's fixed in v1.67 but I leave
            //  I set 2037 as the maximum by now
            auto upper_bound = ptime(date(2037, Dec, 31));
            v.data = getPtimeToPosixStr(upper_bound);
            v_arr.push_back(v);
            continue;
        }

        if (values.find(c.getName()) == values.end()) {
            if ((c.getConstraints().find(ColumnConstraint::primary_key) !=
                 c.getConstraints().end()) ||
                (c.getConstraints().find(ColumnConstraint::not_null) !=
                 c.getConstraints().end())) {
                e.reset(new exc::constr::NullNotNull(name, c.getName()));
                return;
            }
            Value v;
            v.data = "0";
            v.is_null = true;
            v_arr.push_back(v);
        } else {
            Value v;
            v.is_null = false;
            v.data = values[c.getName()];
            v_arr.push_back(v);
        }
    }

    auto tbl_cols = table.getColumns();
    cursor.reset();
    while (cursor.next()) {
        auto f = cursor.fetch();
        for (int i = 0; i < f.size(); ++i) {
            if (f[i].data == v_arr[i].data &&
                ((column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .find(ColumnConstraint::primary_key) !=
                  column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .end()) ||
                 (column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .find(ColumnConstraint::unique) !=
                  column_info[name][tbl_cols[i].getName()]
                      .getConstraints()
                      .end()))) {
                auto dat = f[i].data;
                if (tbl_cols[i].getType() == DataType::varchar) {
                    dat = "null";
                }
                e.reset(new exc::constr::DuplicatedUnique(
                    name, table.getColumns()[i].getName(), dat));
                return;
            }
        }
    }

    if (!e) {
        cursor.insert(v_arr);
    }
}

void QueryManager::update(const Query& query, t_ull transact_num,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    std::string name = query.getChildren()[NodeType::ident]->getName();
    auto table = Engine::show(name);

    std::map<std::string, std::map<std::string, Column>> column_info;
    int sys_end_ind = 0;
    int cnt = 0;
    for (auto& c : table.getColumns()) {
        if (c.getPeriod() == PeriodState::sys_end) {
            sys_end_ind = cnt;
        }
        column_info[table.getName()][c.getName()] = c;
        ++cnt;
    }

    auto idents =
        static_cast<IdentList*>(query.getChildren()[NodeType::ident_list])
            ->getIdents();
    std::set<std::string> col_set;
    for (auto& c : idents) {
        if (column_info[name].find(c->getName()) == column_info[name].end()) {
            e.reset(new exc::acc::ColumnNonexistent(c->getName(), name));
            return;
        }

        if (col_set.find(c->getName()) == col_set.end()) {
            col_set.insert(c->getName());
        } else {
            e.reset(new exc::RepeatColumn(c->getName()));
            return;
        }
    }

    auto constants =
        static_cast<ConstantList*>(query.getChildren()[NodeType::constant_list])
            ->getConstants();

    std::map<std::string, std::string> values;
    for (size_t i = 0; i < constants.size(); ++i) {
        auto id_name = idents[i]->getName();
        if (column_info[name][id_name].getPeriod() != PeriodState::none) {
            e.reset(new exc::UnableToAssignPeriodField(id_name));
            return;
        }
        if (Resolver::compareTypes(name, name, column_info, idents[i],
                                   constants[i], e, CompareCondition::assign,
                                   "=")) {
            if (column_info[name][id_name].getType() == DataType::varchar &&
                column_info[name][id_name].getN() <
                    static_cast<Constant*>(constants[i])->getValue().length()) {
                e.reset(new exc::DataTypeOversize(id_name));
                return;
            }
            values[id_name] = static_cast<Constant*>(constants[i])->getValue();
        } else {
            return;
        }
    }

    std::vector<int> unique_pos;
    int cntr = 0;
    for (auto& c : table.getColumns()) {
        if (c.getConstraints().find(ColumnConstraint::not_null) !=
                c.getConstraints().end() ||
            c.getConstraints().find(ColumnConstraint::primary_key) !=
                c.getConstraints().end() ||
            c.getConstraints().find(ColumnConstraint::unique) !=
                c.getConstraints().end()) {
            unique_pos.push_back(cntr);
        }
        ++cntr;
    }

    Cursor cursor(transact_num, name);
    Cursor history_cursor(transact_num, getHistoryName(name));
    cursor.markUpdate(true);
    std::vector<std::vector<Value>> ready_ftch;
    std::vector<std::vector<Value>> updated_records;
    while (cursor.next()) {
        auto ftch = cursor.fetch();
        std::vector<Value> rec;
        for (int i = 0; i < table.getColumns().size(); ++i) {
            auto c = table.getColumns()[i];
            if (values.find(c.getName()) != values.end()) {
                Value v;
                v.is_null = false;
                auto tmp = values[c.getName()];
                if (tmp == "null") {
                    if (c.getType() == DataType::varchar) {
                        v.data = "0";
                    } else {
                        v.data = "null";
                    }
                    v.is_null = true;
                } else {
                    v.data = values[c.getName()];
                }
                rec.push_back(v);
            } else {
                if (c.getPeriod() == PeriodState::sys_start) {
                    Value v;
                    v.is_null = false;
                    auto curr = boost::posix_time::second_clock::local_time();
                    v.data = getPtimeToPosixStr(curr);
                    rec.push_back(v);
                } else if (c.getPeriod() == PeriodState::sys_end) {
                    Value v;
                    v.is_null = false;
                    auto upper_bound = ptime(date(2037, Dec, 31));
                    v.data = getPtimeToPosixStr(upper_bound);
                    rec.push_back(v);
                } else {
                    rec.push_back(ftch[i]);
                }
            }
        }
        updated_records.push_back(rec);
        ready_ftch.push_back(ftch);
    }

    int f_cnt = 0;
    int u_cnt = 0;

    cursor.reset();
    while (cursor.next()) {
        auto f = cursor.fetch();
        u_cnt = 0;
        for (auto& rec : updated_records) {
            for (auto& u : unique_pos) {
                if (f[u].data == rec[u].data && u_cnt != f_cnt) {
                    e.reset(new exc::constr::DuplicatedUnique(
                        name, table.getColumns()[u].getName(), f[u].data));
                    return;
                }
                if (rec[u].is_null &&
                    table.getColumns()[u].getConstraints().find(
                        ColumnConstraint::not_null) !=
                        table.getColumns()[u].getConstraints().end()) {
                    e.reset(new exc::constr::NullNotNull(
                        name, table.getColumns()[u].getName()));
                    return;
                }
            }
            ++u_cnt;
        }
        ++f_cnt;
    }

    cursor.reset();
    std::set<std::string> repeated;

    while (cursor.next()) {
        auto f = cursor.fetch();
        std::string resp = "0";
        for (int k = 0; k < ready_ftch.size(); ++k) {
            bool equal = true;
            for (int i = 0; i < ready_ftch[k].size(); ++i) {
                if (f[i].data != ready_ftch[k][i].data) {
                    equal = false;
                    break;
                }
            }
            if (!equal) {
                continue;
            }
            std::map<std::string, std::map<std::string, std::string>> record;
            std::map<std::string, std::string> m =
                Resolver::getRecordMap(table.getColumns(), f, e);
            if (e) {
                return;
            }
            record[name] = m;

            auto root = static_cast<Expression*>(
                query.getChildren()[NodeType::expression]);
            resp = Resolver::resolve(name, name, column_info, root, record, e);
            if (e) {
                return;
            }
            if (resp != "0") {
                for (auto& u : unique_pos) {
                    if (repeated.find(updated_records[k][u].data) ==
                        repeated.end()) {
                        repeated.insert(updated_records[k][u].data);
                    } else {
                        e.reset(new exc::constr::DuplicatedUnique(
                            name, table.getColumns()[u].getName(),
                            updated_records[k][u].data));
                        return;
                    }
                }
                if (table.isSystemVersioning()) {
                    insertTemporalTable(name, table.getColumns(), transact_num,
                                        sys_end_ind, f, e);
                }

                cursor.update(updated_records[k]);
            }
            break;
        }
    }
    cursor.markUpdate(false);
}

void QueryManager::remove(const Query& query, t_ull transact_num,
                          std::unique_ptr<exc::Exception>& e,
                          std::ostream& out) {
    auto name = query.getChildren()[NodeType::ident]->getName();
    auto table = Engine::show(name);

    std::map<std::string, std::map<std::string, Column>> column_info;
    int sys_end_ind = 0;
    int cnt = 0;
    for (auto& c : table.getColumns()) {
        if (c.getPeriod() == PeriodState::sys_end) {
            sys_end_ind = cnt;
        }
        column_info[table.getName()][c.getName()] = c;
        ++cnt;
    }

    Cursor cursor(transact_num, name);
    while (cursor.next()) {
        auto ftch = cursor.fetch();
        std::map<std::string, std::map<std::string, std::string>> record;
        std::map<std::string, std::string> m =
            Resolver::getRecordMap(table.getColumns(), ftch, e);
        if (e) {
            return;
        }
        record[name] = m;
        auto root =
            static_cast<Expression*>(query.getChildren()[NodeType::expression]);
        std::string resp =
            Resolver::resolve(name, name, column_info, root, record, e);
        if (e) {
            return;
        }
        if (resp != "0") {
            if (table.isSystemVersioning()) {
                insertTemporalTable(name, table.getColumns(), transact_num,
                                    sys_end_ind, ftch, e);
            }
            cursor.remove();
        }
    }
}

Table QueryManager::resolveRelationalOperTree(
    RelExpr* root, t_ull transact_num, std::unique_ptr<exc::Exception>& e) {
    if (e) {
        return Table();
    }

    if (root && root->childs()[0] && root->childs()[1]) {
        auto child1 = root->childs()[0];
        auto child2 = root->childs()[1];

        Table table1;
        Table table2;

        if (child1->getRelOperType() == RelOperNodeType::table_ident) {
            table1 = getFilledTable(child1->getName(), transact_num, e);
            if (!child1->getAlias().empty()) {
                table1.setName(child1->getAlias());
            }
            if (e) {
                return Table();
            }
        } else {
            table1 = resolveRelationalOperTree(child1, transact_num, e);
            if (e) {
                return Table();
            }
        }

        if (child2->getRelOperType() == RelOperNodeType::table_ident) {
            table2 = getFilledTable(child2->getName(), transact_num, e);
            if (!child2->getAlias().empty()) {
                table2.setName(child2->getAlias());
            }
            if (e) {
                return Table();
            }
        } else {
            table2 = resolveRelationalOperTree(child2, transact_num, e);
            if (e) {
                return Table();
            }
        }

        Table res_table;
        if (root->getRelOperType() == RelOperNodeType::union_) {
            res_table = Union::makeUnion(table1, table2, e);
        } else if (root->getRelOperType() == RelOperNodeType::intersect) {
            res_table = Intersect::makeIntersect(table1, table2, e);
        } else {
            res_table = Join::makeJoin(table1, table2, root->getOnExpr(), e,
                                       root->getRelOperType());
        }

        if (e) {
            return Table();
        }

        res_table.setName(root->getAlias());

        if (e) {
            return Table();
        }

        return res_table;
    }
}

Table QueryManager::getFilledTable(const std::string& name, t_ull transact_num,
                                   std::unique_ptr<exc::Exception>& e) {
    if (!Engine::exists(name)) {
        e.reset(new exc::acc::TableNonexistent(name));
        return Table();
    }
    auto table = Engine::show(name);
    Cursor cursor(transact_num, name);
    if (name == Engine::kTransactionsEndTimesTable) {
        transact_num = 2;
    }

    while (cursor.next()) {
        table.addRecord(cursor.fetch(), e);
        if (e) {
            return Table();
        }
    }

    return table;
}

Table QueryManager::getFilledTempTable(const std::string& name,
                                       t_ull transact_num, const SysTime& stime,
                                       std::unique_ptr<exc::Exception>& e) {
    auto table = Engine::show(name);
    e.reset(nullptr);
    if (!table.isSystemVersioning()) {
        e.reset(new exc::TableIsNotTemporal());
        return Table();
    }

    auto unioned = Union::makeUnion(
        getFilledTable(name, transact_num, e),
        getFilledTable(getHistoryName(name), transact_num, e), e);
    if (e) {
        return Table();
    }
    Expression* root = nullptr;
    auto columns = table.getColumns();
    int sys_start_ind = 0;
    for (int i = 0; i < columns.size(); ++i) {
        if (columns[i].getPeriod() == PeriodState::sys_start) {
            sys_start_ind = i;
            break;
        }
    }
    auto name1 = unioned.getName();
    auto name2 = Engine::kTransactionsEndTimesTable;
    auto child1 = new Expression(
        new Ident(unioned.getColumns()[sys_start_ind].getName()));
    auto child2 = new Expression(new Ident("end_time"));
    root = new Expression(child1, ExprUnit::equal, child2);
    auto joined_with_tr = Join::makeJoin(
        unioned,
        getFilledTable(Engine::kTransactionsEndTimesTable, transact_num, e),
        root, e);

    if (e) {
        return Table();
    }

    return joined_with_tr;
}

std::string QueryManager::getHistoryName(const std::string& name) {
    return name + "History";
}
void QueryManager::insertTemporalTable(const std::string& name,
                                       const std::vector<Column>& cols,
                                       t_ull transact_num,
                                       const int sys_end_ind,
                                       std::vector<Value> rec,
                                       std::unique_ptr<exc::Exception>& e) {
    Cursor cursor(transact_num, getHistoryName(name));
    Value v;
    v.is_null = false;
    auto curr = boost::posix_time::second_clock::local_time();
    v.data = getPtimeToPosixStr(curr);
    rec[sys_end_ind] = v;

    cursor.insert(rec);
}

void QueryManager::createIndex(const Query& query, t_ull transact_num,
                               std::unique_ptr<exc::Exception>& e,
                               std::ostream& out) {
    auto index = static_cast<IndexNode*>(query.getChildren()[NodeType::index]);
    auto table_name = index->getTableName();
    auto column_name = index->getColumnName();

    if (!Engine::exists(table_name)) {
        e.reset(new exc::acc::TableNonexistent(table_name));
        return;
    }
    auto table = Engine::show(table_name);
    auto columns = table.getColumns();
    int col_ind = 0;
    e.reset(new exc::acc::ColumnNonexistent(column_name, table_name));
    for (int i = 0; i < columns.size(); ++i) {
        if (columns[i].getName() == column_name) {
            col_ind = i;
            e.reset(nullptr);
        }
    }
    if (e) {
        return;
    }

    Cursor::createIndex(table, col_ind);
}
std::pair<std::multimap<std::string, int>::iterator,
          std::multimap<std::string, int>::iterator>
QueryManager::getRecordsFromIndex(const std::string& table_name,
                                  Expression* root) {
    Index* index = IndexesManager::get(table_name, 0);
    auto positions = index->getPositions();
    auto child1 = root->childs()[0];
    auto child2 = root->childs()[1];
    NodeType type1 = child1->getConstant()->getNodeType();
    NodeType type2 = child2->getConstant()->getNodeType();

    ExprUnit oper = root->exprType();
    std::string value = nullptr;
    if (type1 == NodeType::constant && type2 == NodeType::ident) {
        value = static_cast<Constant*>(child1->getConstant())->getValue();
        switch (oper) {
            case ExprUnit::greater:
                oper = ExprUnit::less;
                break;
            case ExprUnit::greater_eq:
                oper = ExprUnit::less_eq;
                break;
            case ExprUnit::less:
                oper = ExprUnit::greater;
                break;
            case ExprUnit::less_eq:
                oper = ExprUnit::greater_eq;
                break;
        }
    } else {
        value = static_cast<Constant*>(child2->getConstant())->getValue();
    }

    switch (oper) {
        case ExprUnit::equal:
            return positions.equal_range(value);
            /*        case ExprUnit::not_equal:
                        std::multimap<std::string, int, Index::cmp> res;
                        auto exclude = positions.equal_range(value);
                        // TODO: подумать
                        return {};
                    case ExprUnit::greater:
                        auto begin = positions.upper_bound(value);
                        auto end = positions.end();
                        return {begin, end};
                    case ExprUnit::greater_eq:
                        break;
                    case ExprUnit::less:
                        begin = positions.lower_bound(value);
                        end = positions.end();
                        return {begin, end};
                    case ExprUnit::less_eq:
                        break;*/
    }
}
