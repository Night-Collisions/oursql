
#ifndef OURSQL_APP_LOGIC_TABLEMANAGER_H_
#define OURSQL_APP_LOGIC_TABLEMANAGER_H_

// An api between parser and table-class

#include "Table.h"

void initTable(char *name);
void addField(char *name, char *type, char *value);

#endif //OURSQL_APP_LOGIC_TABLEMANAGER_H_
