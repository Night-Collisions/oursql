#ifndef OURSQL_APP_LOGIC_PARSER_NODES_COMMAND_H_
#define OURSQL_APP_LOGIC_PARSER_NODES_COMMAND_H_

#include <string>
#include "Node.h"

enum class CommandType : unsigned int {
    unknown,
    create_table,
    show_create_table,
    drop_table,
    select,
    Count
};

class Command : public Node {
   public:
    explicit Command(CommandType type)
        : Node(NodeType::command), commandType_(type) {}

    CommandType getCommandType() { return commandType_; }

   private:
    CommandType commandType_;
};

#endif  // OURSQL_APP_LOGIC_PARSER_NODES_COMMAND_H_
