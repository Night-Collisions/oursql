#ifndef OURSQL_APP_LOGIC_TABLE_H_
#define OURSQL_APP_LOGIC_TABLE_H_

#include <string>
#include <map>
#include <vector>
#include "Field.h"

class Table {
  public:

    void setName(std::string name) { name_ = name; }
    void addField(const Field &field);

  private:
    std::string name_;
    std::vector<Field> fields_;
};

#endif
