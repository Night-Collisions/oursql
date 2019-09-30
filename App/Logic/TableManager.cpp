#include "TableManager.h"

#include <iostream>
#include <map>

static Table* table = nullptr;

void initTable(char* name) {
    destroyTable();
    table = new Table();
    table->setName(std::string(name));
}

DataType string2Type(const std::string& s) {
    std::string type = s;
    for (auto& i : type) {
        i = std::tolower(i);
    }
    const std::map<std::string, DataType> map = {{"int", DataType::integer},
                                                 {"real", DataType::real},
                                                 {"text", DataType::text}};
    try {
        return map.at(type);
    } catch (...) {
        return DataType::text;
    }
}

void addField(char* name, char* type, char* value) {
    Field f((std::string(name)), string2Type(std::string(type)),
            std::string(value));
    table->addField(f);
}

void destroyTable() { delete table; }

Table getTable() { return *table; }