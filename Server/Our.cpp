#include "Our.h"

#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

#define EXCEPTION_OURSQL_CHECK(e, out, command)                             \
    if (e != nullptr) {                                                     \
        out << e->getMessage() << "\n"                                      \
            << "~~Exception in command:\"" << command << "\"" << std::endl; \
        delete query;                                                       \
        return e->getNumber();                                              \
    };

namespace ourSQL {

bool get_command(std::istream& in, std::string& command) {
    int c = 0;
    command.clear();
    bool in_quotes = false;
    while ((c = in.get()) != EOF) {
        command.push_back(c);
        if (command.back() == ';' && !in_quotes) {
            return true;
        } else if ((command.back() == '\"' || command.back() == '\'') &&
                   (command.size() < 2 ||
                    command[command.size() - 2] != '\\')) {
            in_quotes = !in_quotes;
        }
    }
    return false;
}

unsigned int perform(std::istream& in, std::ostream& out) {
    std::unique_ptr<exc::Exception> e = nullptr;
    std::string command;
    bool is_end = false;
    do {
        is_end = !get_command(in, command);
        if (command.empty() || command == "\n") {
            return 0;
        }
        ParserManager pm;
        //pm.setDebug(1);

        auto query = pm.getParseTree(command, e);
        EXCEPTION_OURSQL_CHECK(e, out, command);
        QueryManager::execute(*query, e, out);
        EXCEPTION_OURSQL_CHECK(e, out, command);
        delete query;
    } while (!is_end);
    return 0;
}

void nonstop_perform(std::istream& in, std::ostream& out) {
    while (!in.eof()) {
        perform(in, out);
    }
}

}  // namespace ourSQL
