#include "Our.h"

#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"
#include <mutex>
#include <thread>
#include <chrono>


#define EXCEPTION_OURSQL_CHECK(e, out, command)                             \
    if (e != nullptr) {                                                     \
        out << e->getMessage() << "\n"                                      \
            << "~~Exception in command:\"" << command << "\"" << std::endl; \
        queries.clear();                                                    \
        return e->getNumber();                                              \
    };

namespace ourSQL {
std::mutex transact_counter_mtx;
volatile unsigned long long transaction_number_ = 0;

size_t contains(const std::string& s, const std::string& key) {
    std::string tmp;
    for (auto& c : s) {
        tmp.push_back(tolower(c));
    }

    return tmp.find(key);
}

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
    return !command.empty();
}

unsigned int perform(std::istream& in, std::ostream& out) {
    std::unique_ptr<exc::Exception> e = nullptr;
    std::string command;
    static bool transact_begin = false;
    static std::map<unsigned long long, unsigned long long> users_transacts;
    static std::map<unsigned long long, bool> users_begins;
    bool is_end = false;
    do {
        // TODO: вынести это и прописать ошибки
        is_end = !get_command(in, command);
        if (command.empty() || command == "\n") {
            return 0;
        }
        if (contains(command, "begin")) {
            std::unique_lock<std::mutex> lock(transact_counter_mtx);
            if (transact_begin) {
                // exception, transaction has already been started
            } else {

                transact_begin = true;
                continue;
            }
        } else if (!transact_begin) {
            std::unique_lock<std::mutex> lock(transact_counter_mtx);
            ++transaction_number_;
        }

        if (contains(command, "commit")) {
            if (transact_begin) {
                // TODO
                // Как то сказать движку, что мы сделали коммит?
                transact_begin = false;
                continue;
            } else {
                // exception: no unfinished transaction
            }
        }

        ParserManager pm;
        // pm.setDebug(1);
        auto queries = pm.getParseTree(command, e);
        EXCEPTION_OURSQL_CHECK(e, out, command);
        for (auto& q : queries) {
            QueryManager::execute(*q, transaction_number_, e, out);
            //TODO: если это единичная команда, то сразу делать коммит
        }

        EXCEPTION_OURSQL_CHECK(e, out, command);
        queries.clear();
    } while (!is_end);
    return 0;
}

void nonstop_perform(std::istream& in, std::ostream& out) {
    while (!in.eof()) {
        perform(in, out);
    }
}

}  // namespace ourSQL