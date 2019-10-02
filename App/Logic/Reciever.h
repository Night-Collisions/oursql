#ifndef OURSQL_APP_LOGIC_RECIEVER_H_
#define OURSQL_APP_LOGIC_RECIEVER_H_

#include "../Engine/Table.h"

void recieveCreate(bool response);

void recieveDrop(bool response);

void recieveShowCreate(Table table);

void recieveShowTables(std::string response);

#endif  // OURSQL_APP_LOGIC_RECIEVER_H_
