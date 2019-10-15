
#include "ConditionChecker.h"

bool ConditionChecker::check(const rapidjson::Value& record) const {
    if (always_true_) {
        return true;
    }

    std::string left_val;
    std::string right_val;

    if (left_nd_ == NodeType::ident) {
        left_val = record[left_].GetString();
        if (left_val == "null") {
            return false;
        }
    } else {
        left_val = left_;
    }

    if (right_nd_ == NodeType::ident) {
        right_val = record[right_].GetString();
        if (right_val == "null") {
            return false;
        }
    } else {
        right_val = right_;
    }

    // TODO: переделат не через свитч

    switch (data_type_) {
        case DataType::integer:
            switch (rel_type_) {
                case RelationType::equal:
                    return std::stoi(left_val) == std::stoi(right_val);
                case RelationType::not_equal:
                    return std::stoi(left_val) != std::stoi(right_val);
                case RelationType::greater:
                    return std::stoi(left_val) > std::stoi(right_val);
                case RelationType::greater_eq:
                    return std::stoi(left_val) >= std::stoi(right_val);
                case RelationType::less:
                    return std::stoi(left_val) < std::stoi(right_val);
                case RelationType::less_eq:
                    return std::stoi(left_val) <= std::stoi(right_val);
            }
        case DataType::real:
            switch (rel_type_) {
                case RelationType::equal:
                    return std::stof(left_val) == std::stof(right_val);
                case RelationType::not_equal:
                    return std::stof(left_val) != std::stof(right_val);
                case RelationType::greater:
                    return std::stof(left_val) > std::stof(right_val);
                case RelationType::greater_eq:
                    return std::stof(left_val) >= std::stof(right_val);
                case RelationType::less:
                    return std::stof(left_val) < std::stof(right_val);
                case RelationType::less_eq:
                    return std::stof(left_val) <= std::stof(right_val);
            }
        case DataType::text:
            switch (rel_type_) {
                case RelationType::equal:
                    return left_val == right_val;
                case RelationType::not_equal:
                    return left_val != right_val;
                case RelationType::greater:
                    return left_val > right_val;
                case RelationType::greater_eq:
                    return left_val >= right_val;
                case RelationType::less:
                    return left_val < right_val;
                case RelationType::less_eq:
                    return left_val <= right_val;
            }
    }

    return false;
}
