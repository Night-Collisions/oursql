#include "Field.h"

void Field::addData(const std::string& data) {
    if (!checkDataForType(type_, data)) {
        throw 1;
    }
    data_.push_back(data);
}

bool checkDataForType(const DataType type, const std::string& data) {
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
            bool foundedPoint = false;
            for (unsigned int i =
                     ((s.size() > 2 && s[0] == '-' && std::isdigit(s[1]))
                          ? (1)
                          : (0));
                 i < s.size(); i++) {
                if (!std::isdigit(s[i])) {
                    if (s[i] == '.') {
                        if (foundedPoint) {
                            return false;
                        }
                        foundedPoint = true;
                    }
                    return false;
                }
            }
            return true;
        },
        [](const std::string& s) { return true; }};
    return isAccording[static_cast<unsigned int>(type)](data);
}