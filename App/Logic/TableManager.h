#ifndef OURSQL_APP_LOGIC_TABLEMANAGER_H_
#define OURSQL_APP_LOGIC_TABLEMANAGER_H_

// An api between parser and table-class

#include "../Engine/Table.h"

void initTable(char *name);
void addField(char *name, char *type, char *value);
void destroyTable();

#endif  // OURSQL_APP_LOGIC_TABLEMANAGER_H_
