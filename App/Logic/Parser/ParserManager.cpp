#include <string>

#include "ParserManager.h"
#include "parser.cpp"

Query* ParserManager::getParseTree(const std::string& query,
                                   std::unique_ptr<exc::Exception>& exception) {
    yydebug = debug_;
    return parse_string(query.c_str(), exception.get());
}

Query* ParserManager::getParseTree(FILE* input, std::unique_ptr<exc::Exception>& exception) {
    yydebug = debug_;

    char c = ' ';
    std::string query;

    while ((c = fgetc(input)) != EOF) {
        if (c != '\n') {
            query += c;
        }

        if (c == ';') {
            break;
        }
    }

    return parse_string(query.c_str(), exception.get());
}
