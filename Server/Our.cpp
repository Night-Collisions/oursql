#include "Our.h"

#include <chrono>
#include <mutex>
#include <thread>

#include "Engine/Engine.h"
#include "Logic/Parser/ParserManager.h"
#include "Logic/QueryManager.h"

std::map<std::string, bool> ourSQL::locked_tables_;

#if defined(queries)
#define EXCEPTION_OURSQL_CHECK(e, out, command)                             \
    if (e != nullptr) {                                                     \
        out << e->getMessage() << "\n"                                      \
            << "~~Exception in command:\"" << command << "\"" << std::endl; \
        queries.clear();                                                    \
        return e->getNumber();                                              \
    };
#else
#define EXCEPTION_OURSQL_CHECK(e, out, command)                             \
    if (e != nullptr) {                                                     \
        out << e->getMessage() << "\n"                                      \
            << "~~Exception in command:\"" << command << "\"" << std::endl; \
        return e->getNumber();                                              \
    };
#endif

namespace ourSQL {
std::mutex transact_counter_mtx;
volatile unsigned long long transaction_number_ = 0;

bool contains(const std::string& s, const std::string& key) {
    std::string tmp;
    for (auto& c : s) {
        tmp.push_back(tolower(c));
    }

    return tmp.find(key) != std::string::npos;
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

unsigned int perform(std::istream& in, std::ostream& out,
                     unsigned short client_id) {
    std::unique_ptr<exc::Exception> e = nullptr;
    std::string command;
    static std::map<unsigned long long, unsigned long long> users_transacts;
    static std::map<unsigned long long, bool> users_begins;
    bool is_end = false;
    do {
        is_end = !get_command(in, command);
        if (command.empty() || command == "\n") {
            return 0;
        }
        if (contains(command, "begin")) {
            std::unique_lock<std::mutex> lock(transact_counter_mtx);
            if (users_begins.find(client_id) != users_begins.end() &&
                users_begins[client_id]) {
                e.reset(new exc::tr::RepeatBeginTransact());
                EXCEPTION_OURSQL_CHECK(e, out, command);
            } else {
                users_begins[client_id] = true;
                users_transacts[client_id] =
                    Engine::generateNextTransactionId();
                Engine::beginTransaction(users_transacts[client_id]);
                continue;
            }
        } else if (!users_begins[client_id]) {
            // it means it's a single command
            std::unique_lock<std::mutex> lock(transact_counter_mtx);
            users_transacts[client_id] = Engine::generateNextTransactionId();
            Engine::beginTransaction(users_transacts[client_id]);
        }

        if (contains(command, "commit")) {
            std::unique_lock<std::mutex> lock(transact_counter_mtx);
            if (users_begins[client_id]) {
                users_begins[client_id] = false;
                Engine::commitTransaction(users_transacts[client_id]);
                continue;
            } else {
                e.reset(new exc::tr::NoUncommitedTransact());
                EXCEPTION_OURSQL_CHECK(e, out, command);
            }
        }

        ParserManager pm;
        // pm.setDebug(1);
        auto queries = pm.getParseTree(command, e);
        EXCEPTION_OURSQL_CHECK(e, out, command);
        for (auto& q : queries) {
            QueryManager::execute(*q, users_transacts[client_id], e, out,
                                  locked_tables_);
            if (!users_begins[client_id]) {
                Engine::commitTransaction(users_transacts[client_id]);
            }
        }

        EXCEPTION_OURSQL_CHECK(e, out, command);
        queries.clear();
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