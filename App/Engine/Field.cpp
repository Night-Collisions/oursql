#include "Field.h"
#include <iostream>

std::map<std::string, FieldConstraint> Field::constraint_map_;

void Field::addData(const std::string& data) {
    if (!checkDataForType(type_, data)) {
        throw 1;
    }
    data_.push_back(data);
}

bool Field::checkDataForType(const DataType type, const std::string& data) {
    bool (*isAccording[static_cast<unsigned int>(DataType::Count)])(
        const std::string&) = {

        [](const std::string& s) {
            for (unsigned int i =
                     ((s.size() > 2 && s[0] == '-' && std::isdigit(s[1]))
                          ? (1)
                          : (0));
                 i < s.size(); i++) {
                if (!std::isdigit(s[i])) {
                    return false;
                }
            }
            return true;
        },
        [](const std::string& s) {
            bool foundPoint = false;
            for (unsigned int i =
                     ((s.size() > 2 && s[0] == '-' && std::isdigit(s[1]))
                          ? (1)
                          : (0));
                 i < s.size(); i++) {
                if (!std::isdigit(s[i])) {
                    if (s[i] == '.') {
                        if (foundPoint) {
                            return false;
                        }
                        foundPoint = true;
                    }
                    return false;
                }
            }
            return true;
        },
        [](const std::string& s) { return true; }};
    return isAccording[static_cast<unsigned int>(type)](data);
}

std::vector<std::string> split(const std::string& s, const char sep) {
    int cnt = 0;
    std::vector<std::string> res;
    res.resize(cnt + 1);

    for (auto& i : s) {
        if (i == sep) {
            res.resize(++cnt);
        } else {
            res[cnt].push_back(i);
        }
    }

    return res;
}

std::set<FieldConstraint> Field::checkConstraints(
    const std::string& constraints) {
    // TODO: проверить, чтобы не было дубликатов констрейнтов


    std::set<FieldConstraint> res;

    auto separated = split(constraints, ' ');

    //std::cout << separated[0];

    for (auto& c : separated) {
        std::cout << c << std::endl;
        if (constraint_map_.find(c) == constraint_map_.end()) {
            // the so-called constraint doesn't exists
            // TODO: throw exception?
        } else {
            res.insert(constraint_map_[c]);
        }
    }

    return res;
}

void Field::initMap() {
    constraint_map_["not_null"] = FieldConstraint::not_null;
    constraint_map_["primary_key"] = FieldConstraint::primary_key;
    constraint_map_["foreign_key"] = FieldConstraint::foreign_key;
    constraint_map_["unique"] = FieldConstraint::unique;
}
