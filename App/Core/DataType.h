#ifndef OURSQL_DATATYPE_H
#define OURSQL_DATATYPE_H

#include <string>

enum class DataType : unsigned int { integer, real, text, Count };

std::string DataType2String(const DataType&);
DataType String2DataType(const std::string&);

#endif //OURSQL_DATATYPE_H
