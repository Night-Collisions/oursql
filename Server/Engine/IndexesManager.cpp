#include "IndexesManager.h"

std::unordered_map<std::string, std::map<int, Index>> IndexesManager::indexes;

bool IndexesManager::exists(const std::string& tablename, int column_index) {
    auto it = indexes.find(tablename);
    if (it == indexes.end()) {
        return false;
    }
    return it->second.find(column_index) != it->second.end();
}

Index* IndexesManager::get(const std::string& tablename, int column_index) {
    return &(indexes.find(tablename)->second.find(column_index)->second);
}

void IndexesManager::create(const Table& table, int column_index) {
    if (indexes.find(table.getName()) == indexes.end()) {
        indexes.insert(std::make_pair(table.getName(), std::map<int, Index>()));
    }
    auto it = indexes.find(table.getName());
    it->second.emplace(column_index, Index(table, column_index));
}

void IndexesManager::drop(const std::string& tablename, int column_index) {
    indexes.find(tablename)->second.erase(column_index);
    if (indexes.find(tablename)->second.empty()) {
        indexes.erase(tablename);
    }
}

void IndexesManager::dropTable(const std::string& tablename) {
    indexes.erase(tablename);
}

void IndexesManager::clear() {
    indexes.clear();
}

std::map<int, Index>* IndexesManager::getTableIndexes(const std::string& tablename) {
    auto it = indexes.find(tablename);
    return (it == indexes.end()) ? (nullptr) : (&(it->second));
}