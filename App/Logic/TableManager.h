
#ifndef OURSQL_APP_LOGIC_TABLEMANAGER_H_
#define OURSQL_APP_LOGIC_TABLEMANAGER_H_

// An api between parser and table-class

#include "Table.h"

static Table *table;

void initTable(char *);
void addField(char *name, char *type, int value);

#endif //OURSQL_APP_LOGIC_TABLEMANAGER_H_
