#ifndef OURSQL_INDEX_H
#define OURSQL_INDEX_H

#include <functional>
#include <map>
#include <string>
#include "Table.h"

class Index {
   public:
    using cmp = std::function<bool(const std::string&, const std::string&)>;

    Index(const Table& table, int column_index) :
        positions_(Comparator(table.getColumns()[column_index].getType())),
        table_(table),
        column_index_(column_index) {}

    std::multimap<std::string, int, cmp>& getPositions() { return positions_; }

    int getIndex() { return column_index_; }
    const Table& getTable() { return table_; }

   private:
    struct Comparator {
        Comparator(DataType dataType) : datatype(dataType) {}

        bool operator() (const std::string& a, const std::string& b) const {
            switch (datatype) {
                case DataType::integer:
                    return std::stoi(a) < std::stoi(b);
                case DataType::datetime:
                    return std::stoull(a) < std::stoull(b);
                case DataType::real:
                    return std::stod(a) < std::stod(b);
                case DataType::varchar: {
                    return a < b;
                }
            }
        }

        DataType datatype;
    };

    std::multimap<std::string, int, cmp> positions_;
    Table table_;
    int column_index_;
};


#endif
