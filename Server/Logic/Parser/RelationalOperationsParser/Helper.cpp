

#include "Helper.h"
#include <vector>
bool Helper::checkTypes(DataType a, DataType b) {
    if (a == DataType::varchar && b == DataType::varchar) {
        return true;
    }

    if (a == DataType::real && b == DataType::integer) {
        return true;
    }

    if (a == DataType::integer && b == DataType::real) {
        return true;
    }

    return a == b;
}

std::string Helper::recordToConcat(const std::vector<Value>& r) {
    std::string res;
    for (auto& c : r) {
        res += c.data;
    }

    return res;
}

std::string Helper::getCorrectTablePrefix(const std::string& s) {
    return (s.empty()) ? ("") : (s + ".");
}
