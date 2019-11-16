#ifndef OURSQL_SERVER_LOGIC_TRANSACTIONMANAGER_H_
#define OURSQL_SERVER_LOGIC_TRANSACTIONMANAGER_H_

#include <bits/unique_ptr.h>
#include "../Core/Exception.h"
#include "Parser/Nodes/Transaction.h"

class TransactionManager {
   public:
    TransactionManager() = delete;

    static void execute(const Transaction& transact,
                        std::unique_ptr<exc::Exception>& e,
                        unsigned long long transact_num, std::ostream& out);
};

#endif  // OURSQL_SERVER_LOGIC_TRANSACTIONMANAGER_H_
