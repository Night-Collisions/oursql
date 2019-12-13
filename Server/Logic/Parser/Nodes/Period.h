#ifndef OURSQL_SERVER_LOGIC_PARSER_NODES_PERIOD_H_
#define OURSQL_SERVER_LOGIC_PARSER_NODES_PERIOD_H_

#include "Node.h"

class Period : public Node {
   public:
    explicit Period(const std::pair<std::string, std::string>& p)
        : Node(NodeType::period_pair), period_(p) {}

    std::pair<std::string, std::string> getPeriod() { return period_; }
    std::string getName() override { return std::string(); }

   private:
    std::pair<std::string, std::string> period_;
};

#endif  // OURSQL_SERVER_LOGIC_PARSER_NODES_PERIOD_H_
