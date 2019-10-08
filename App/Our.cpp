#include "Our.h"

#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

#define EXCEPTION_OURSQL_CHECK(e, out)       \
    if (e != nullptr) {                      \
        out << e->getMessage() << std::endl; \
        return e->getNumber();               \
    };

namespace ourSQL {

std::string get_command(std::istream& in) {
    std::string ans;
    while (!in.eof()) {
        ans.push_back(in.get());
        if (ans.back() == ';') {
            return ans;
        }
    }
    return ans;
}

unsigned int perform(std::istream& in, std::ostream& out) {
    std::unique_ptr<exc::Exception> e = nullptr;
    while (!in.eof()) {
        ParserManager pm;
        auto a = pm.getParseTree(get_command(in), e);
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
