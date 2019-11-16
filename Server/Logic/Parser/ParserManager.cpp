#include <string>

#include "ParserManager.h"
#include "parser.cpp"

Transaction* ParserManager::getParseTree(const std::string& query,
                                   std::unique_ptr<exc::Exception>& exception) {
    yydebug = debug_;
    return parse_string(query.c_str(), exception);
}
