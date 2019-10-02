#include "Field.h"

#include <array>
#include <map>
#include <iostream>

//---DataType---//
std::array<std::string, static_cast<unsigned int>(DataType::Count)>
    DataType2Names = {"int", "real", "text"};
std::map<std::string, DataType> Name2DataType = {{DataType2Names[0], DataType::integer},
                                                 {DataType2Names[1], DataType::real},
                                                 {DataType2Names[2], DataType::text}};

std::string DataType2String(const DataType& type) {
    return DataType2Names[static_cast<unsigned int>(type)];
}

DataType String2DataType(const std::string& s) { return Name2DataType.at(s); }
//------//

//---FieldConstraint---//
std::array<std::string, static_cast<unsigned int>(FieldConstraint::Count)>
    FieldConstraint2Names = {"primary key", "foreign key", "not null", "unique"};
std::map<std::string, FieldConstraint> Name2FieldConstraint = {{FieldConstraint2Names[0], FieldConstraint::primary_key},
                                                 {FieldConstraint2Names[1], FieldConstraint::foreign_key},
                                                 {FieldConstraint2Names[2], FieldConstraint::not_null},
                                                 {FieldConstraint2Names[3], FieldConstraint::unique}};

std::string FieldConstraint2String(const FieldConstraint& c) {
    return FieldConstraint2Names[static_cast<unsigned int>(c)];
}

FieldConstraint String2FieldConstraint(const std::string& s) { return Name2FieldConstraint.at(s); }
//------//

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

    // std::cout << separated[0];

    for (auto& c : separated) {
        std::cout << c << std::endl;
        if (Name2FieldConstraint.find(c) == Name2FieldConstraint.end()) {
            // the so-called constraint doesn't exists
            // TODO: throw exception?
        } else {
            res.insert(Name2FieldConstraint[c]);
        }
    }

    return res;
}