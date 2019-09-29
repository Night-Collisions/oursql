#include <iostream>
#include "TableManager.h"

static Table *table;

void initTable(char *name) {
    table = new Table();
    table->setName(std::string(name));
}

void addField(char *name, char *type, char *value) {
    Field f((std::string(name)), std::string(type), std::string(value));
    table->addField(f);
}

void destroyTable();