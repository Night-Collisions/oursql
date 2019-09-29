#include <iostream>
#include "TableManager.h"

void initTable(char *name) {
    table = new Table();
    table->setName(std::string(name));
}

void addField(char *name, char *type, int value) {
    Field f(std::string(name), Types::INT_, value);
    table->addField(f);
}

void destroyTable();