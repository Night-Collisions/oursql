/*
#ifndef OURSQL_APP_LOGIC_TABLEMANAGER_H_
#define OURSQL_APP_LOGIC_TABLEMANAGER_H_

// An api between parser and table-class

#include "../Engine/Table.h"

void initTable(char* name);
void addField(char* name, char* type, char* constraints);
void destroyTable();

const char* showCreateTable(const std::string& response);
void dropTable(const char* name);
Table getTable();

#endif  // OURSQL_APP_LOGIC_TABLEMANAGER_H_
*/
