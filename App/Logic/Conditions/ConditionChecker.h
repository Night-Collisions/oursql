//
// Created by Victor on 10.10.2019.
//

#ifndef OURSQL_APP_LOGIC_CONDITIONS_CONDITIONCHECKER_H_
#define OURSQL_APP_LOGIC_CONDITIONS_CONDITIONCHECKER_H_

#define RAPIDJSON_HAS_STDSTRING 1

#include <rapidjson/document.h>
#include "../Parser/Nodes/Constant.h"
#include "../Parser/Nodes/Node.h"

class ConditionChecker {
   public:
   [[deprecated]] ConditionChecker(bool alwaysTrue) : always_true_(alwaysTrue) {}

    [[nodiscard]] bool check(const rapidjson::Value& record) const;

   private:
    std::string left_;
    std::string right_;
    DataType data_type_;
    NodeType left_nd_;
    NodeType right_nd_;

    bool always_true_ = false;
};

#endif  // OURSQL_APP_LOGIC_CONDITIONS_CONDITIONCHECKER_H_
