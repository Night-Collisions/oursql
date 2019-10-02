#include "Field.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <iostream>
#include <map>
#include <set>

//---DataType---//
std::array<std::string, static_cast<unsigned int>(DataType::Count)>
    DataType2Names = {"int", "real", "text"};
std::map<std::string, DataType> Name2DataType = {
    {DataType2Names[0], DataType::integer},
    {DataType2Names[1], DataType::real},
    {DataType2Names[2], DataType::text}};

std::string DataType2String(const DataType& type) {
    return DataType2Names[static_cast<unsigned int>(type)];
}

DataType String2DataType(const std::string& s) { return Name2DataType.at(s); }
//------//

//---FieldConstraint---//
std::array<std::string, static_cast<unsigned int>(FieldConstraint::Count)>
    FieldConstraint2Names = {"primary_key", "not_null", "unique"};
std::map<std::string, FieldConstraint> Name2FieldConstraint = {
    {FieldConstraint2Names[0], FieldConstraint::primary_key},
    {FieldConstraint2Names[1], FieldConstraint::not_null},
    {FieldConstraint2Names[2], FieldConstraint::unique}};

std::string FieldConstraint2String(const FieldConstraint& c) {
    return FieldConstraint2Names[static_cast<unsigned int>(c)];
}

FieldConstraint String2FieldConstraint(const std::string& s) {
    return Name2FieldConstraint.at(s);
}
//------//

void Field::addData(const std::string& data) {
    if (!checkDataForType(type_, data)) {
        throw std::invalid_argument("Data type mismatch!");
    }
    data_.push_back(data);
}

void Field::checkConstraint(const std::set<FieldConstraint>& constraint) {
    std::array<std::set<FieldConstraint>,
               static_cast<unsigned int>(FieldConstraint::Count)>
        incompatible = {std::set<FieldConstraint>{},
                        std::set<FieldConstraint>{},
                        std::set<FieldConstraint>{}};
    for (const auto& i : constraint) {
        std::set<FieldConstraint> buff;
        std::set_intersection(
            incompatible[static_cast<unsigned int>(i)].begin(),
            incompatible[static_cast<unsigned int>(i)].end(),
            constraint.begin(), constraint.end(),
            std::inserter(buff, buff.begin()));
        if (buff.size() > 0) {
            throw std::invalid_argument("Incompatible constraints!");
        }
    }
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

std::set<FieldConstraint> Field::checkConstraints(
    const std::string& constraints) {
    // TODO: проверить, чтобы не было дубликатов констрейнтов

    std::set<FieldConstraint> res;

    auto separated = split(constraints, ' ');

    for (auto& c : separated) {
        if (Name2FieldConstraint.find(c) == Name2FieldConstraint.end()) {
            // the so-called constraint doesn't exists
            // TODO: throw exception?
        } else {
            auto constraint = Name2FieldConstraint[c];
            if (res.find(constraint) == res.end()) {
                throw std::invalid_argument("Identical constraints!");
            }
            res.insert(constraint);
        }
    }

    return res;
}