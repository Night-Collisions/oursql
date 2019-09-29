#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <string>
#include "Types.h"

class Field {
  public:
    Field(std::string name, Types type, int value) : name_(name),
                                                        type_(type),
                                                        iVal_(value) {}
    Field(std::string name, Types type, double value) : name_(name),
                                                           type_(type),
                                                           dVal_(value) {}
/*    Field(std::string &name, Types type, char * value) : name_(name),
                                                                type_(type),
                                                                tVal_(value) {}*/

    void getVal(void *dst);
    Types getType() { return type_; };
    std::string getName() { return name_; };

  private:
    std::string name_;
    Types type_;
    int iVal_{};
    double dVal_{};
    //char *tVal_;
};

#endif