#include "TableManager.h"
#include "../Engine/Engine.h"

#include <iostream>
#include <map>

static Table* table = nullptr;

void initTable(char* name) {
    destroyTable();
    table = new Table();
 //   if (!exists(std::string(name))) {
        table->setName(std::string(name));
   // } else {
        //TODO: сказать, что такая таблица уже существует
  //  }
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

void addField(char* name, char* type, char* constraints) {
    auto s = Field::checkConstraints(std::string(constraints));
   // if (!table->fieldExists(std::string(name))) {
        Field f((std::string(name)), string2Type(std::string(type)), s);
        table->addField(f);
   // } else {
        //TODO: такая переменная уже есть
   // }
}

void destroyTable() { delete table; }

Table getTable() { return *table; }