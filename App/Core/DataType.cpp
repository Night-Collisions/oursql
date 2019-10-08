#include "DataType.h"

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
