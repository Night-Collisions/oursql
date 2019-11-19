#include "Our.h"

#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

#define EXCEPTION_OURSQL_CHECK(e, out, command)                             \
    if (e != nullptr) {                                                     \
        out << e->getMessage() << "\n"                                      \
            << "~~Exception in command:\"" << command << "\"" << std::endl; \
        transacts.clear();                                                  \
        return e->getNumber();                                              \
    };

namespace ourSQL {
volatile unsigned long long transaction_number_ = 0;

size_t containsBegin(const std::string& s) {
    std::string tmp;
    for (auto& c : s) {
        tmp.push_back(tolower(c));
    }

    return tmp.find("begin");
}

bool get_command(std::istream& in, std::string& command) {
    int c = 0;
    command.clear();
    bool in_quotes = false;
    bool no_split = false;
    while ((c = in.get()) != EOF) {
        command.push_back(c);
        if (command.back() == ';' && !in_quotes) {
            if (!no_split && containsBegin(command) != std::string::npos) {
                no_split = true;
            } else if (!no_split) {
                return true;
            }
        } else if ((command.back() == '\"' || command.back() == '\'') &&
                   (command.size() < 2 ||
                    command[command.size() - 2] != '\\')) {
            in_quotes = !in_quotes;
        }
    }
    return !command.empty();
}

unsigned int perform(std::istream& in, std::ostream& out,
                     unsigned short client_id) {
    std::unique_ptr<exc::Exception> e = nullptr;
    std::string command;
    bool is_end = false;
    do {
        // если у нас начинается с begin; то не сплитим
        is_end = !get_command(in, command);
        if (command.empty() || command == "\n") {
            return 0;
        }
        ParserManager pm;
        // pm.setDebug(1);

        auto transacts =
            pm.getParseTree(command, e);  // теперь возвращает вектор
        EXCEPTION_OURSQL_CHECK(e, out, command);
        for (auto& t : transacts) {
            auto transact_num = transaction_number_++;
            for (auto q : t->getChildren()) {
                QueryManager::execute(*q, transact_num, e, out);
            }
        }
        EXCEPTION_OURSQL_CHECK(e, out, command);
        transacts.clear();
    } while (!is_end);
    return 0;
}

void nonstop_perform(std::istream& in, std::ostream& out,
                     unsigned short client_id) {
    while (!in.eof()) {
        perform(in, out, client_id);
    }
}

void forget_client(unsigned short client_id) {
    std::cout << "Forget: " << client_id << std::endl;
}

}  // namespace ourSQL