//
// Created by Victor on 10.10.2019.
//

#ifndef OURSQL_APP_LOGIC_CONDITIONS_CONDITIONCHECKER_H_
#define OURSQL_APP_LOGIC_CONDITIONS_CONDITIONCHECKER_H_

#define RAPIDJSON_HAS_STDSTRING 1

#include <rapidjson/document.h>
#include "../Parser/Nodes/Constant.h"
#include "../Parser/Nodes/Node.h"
#include "../Parser/Nodes/Relation.h"

class ConditionChecker {
   public:
    ConditionChecker(std::string left, std::string right, NodeType leftNd,
                   NodeType rightNd, RelationType rel, DataType dataType)
        : left_(left),
          right_(right),
          left_nd_(leftNd),
          right_nd_(rightNd),
          rel_type_(rel),
          data_type_(dataType) {}

    bool check(const rapidjson::Value& record) const;

   private:
    std::string left_;
    std::string right_;
    RelationType rel_type_;
    DataType data_type_;
    NodeType left_nd_;
    NodeType right_nd_;
};

#endif  // OURSQL_APP_LOGIC_CONDITIONS_CONDITIONCHECKER_H_
