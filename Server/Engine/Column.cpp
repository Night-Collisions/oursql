#include "Column.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

//---ColumnConstraint---//
std::array<std::string, static_cast<unsigned int>(ColumnConstraint::Count)>
    ColumnConstraint2Names = {"primary key", "not null", "unique"};
std::map<std::string, ColumnConstraint> Name2ColumnConstraint = {
    {ColumnConstraint2Names[0], ColumnConstraint::primary_key},
    {ColumnConstraint2Names[1], ColumnConstraint::not_null},
    {ColumnConstraint2Names[2], ColumnConstraint::unique}};

std::string ColumnConstraint2String(const ColumnConstraint& c) {
    return ColumnConstraint2Names[static_cast<unsigned int>(c)];
}

std::string ColumnConstraint2String(unsigned char constraint) {
    for (auto& c : Column::bitConstraintToSet(constraint)) {
        return ColumnConstraint2String(c);
    }
    return std::string();
}

ColumnConstraint String2ColumnConstraint(const std::string& s) {
    return Name2ColumnConstraint.at(s);
}
//------//

void Column::addData(const std::string& data,
                     std::unique_ptr<exc::Exception>& e) {
    e.reset(nullptr);
    if (!checkDataForType(type_, data)) {
        e.reset(new exc::SetDataTypeMismatch(type_, data));
        return;
    }
    data_.push_back(data);
}

bool Column::checkConstraint(
    const std::set<ColumnConstraint>& constraint,
    std::pair<ColumnConstraint, ColumnConstraint>& constraints) {
    std::array<std::set<ColumnConstraint>,
               static_cast<unsigned int>(ColumnConstraint::Count)>
        incompatible = {std::set<ColumnConstraint>{},
                        std::set<ColumnConstraint>{},
                        std::set<ColumnConstraint>{}};
    for (const auto& i : constraint) {
        std::set<ColumnConstraint> buff;
        std::set_intersection(
            incompatible[static_cast<unsigned int>(i)].begin(),
            incompatible[static_cast<unsigned int>(i)].end(),
            constraint.begin(), constraint.end(),
            std::inserter(buff, buff.begin()));
        if (buff.size() > 0) {
            constraints = std::make_pair(i, *buff.begin());
            return true;
        }
    }
    return false;
}

bool Column::checkDataForType(const DataType type, const std::string& data) {
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
            bool found_point = false;
            for (unsigned int i =
                     ((s.size() > 2 && s[0] == '-' && std::isdigit(s[1]))
                          ? (1)
                          : (0));
                 i < s.size(); i++) {
                if (!std::isdigit(s[i])) {
                    if (s[i] == '.') {
                        if (found_point) {
                            return false;
                        }
                        found_point = true;
                    }
                    return false;
                }
            }
            return true;
        },
        [](const std::string& s) { return true; }};
    return isAccording[static_cast<unsigned int>(type)](data);
}

void Column::addValue(const Value& v) noexcept {
    values_.push_back(v);
}

std::vector<std::string> split(const std::string& s, const char sep) {
    int siz = 1;
    std::vector<std::string> res;
    res.resize(siz);

    for (auto& i : s.substr(0, s.size() - 1)) {
        if (i == sep) {
            res.resize(++siz);
        } else {
            res[siz - 1].push_back(i);
        }
    }

    return res;
}
