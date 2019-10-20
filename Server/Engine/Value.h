#ifndef OURSQL_VALUE_H
#define OURSQL_VALUE_H

#include <string>

struct Value {
    std::string data;
    bool is_null = false;
};


#endif
