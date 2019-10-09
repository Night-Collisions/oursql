#include "Our.h"

#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

#define EXCEPTION_OURSQL_CHECK(e, out)       \
    if (e != nullptr) {                      \
        out << e->getMessage() << std::endl; \
        delete a;                            \
        return e->getNumber();               \
    };

namespace ourSQL {

bool get_command(std::istream& in, std::string& command) {
    int c = 0;
    command.clear();
    while ((c = in.get()) != EOF) {
        command.push_back(c);
        if (command.back() == ';') {
            return true;
        }
    }
    return false;
}

unsigned int perform(std::istream& in, std::ostream& out) {
    std::unique_ptr<exc::Exception> e = nullptr;
    std::string command;
    while (get_command(in, command)) {
        ParserManager pm;
        auto a = pm.getParseTree(command, e);
        EXCEPTION_OURSQL_CHECK(e, out);
        QueryManager::execute(*a, e);
        EXCEPTION_OURSQL_CHECK(e, out);
        delete a;
    }
    return 0;
}

void nonstop_perform(std::istream& in, std::ostream& out) {
    while (!in.eof()) {
        perform(in, out);
    }
}

}  // namespace ourSQL
