#ifndef OURSQL_APP_LOGIC_FIELD_H_
#define OURSQL_APP_LOGIC_FIELD_H_

#include <string>
#include <utility>

class Field {
   public:
    Field(std::string name, std::string type, std::string value)
        : name_(std::move(name)),
          type_(std::move(type)),
          val_(std::move(value)) {}

    std::string getVal() { return val_; };
    std::string getType() { return type_; };
    std::string getName() { return name_; };

   private:
    std::string name_;
    std::string type_;
    std::string val_;
};

#endif