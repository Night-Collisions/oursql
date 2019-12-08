#ifndef OURSQL_REQUESTGENERATOR_H
#define OURSQL_REQUESTGENERATOR_H

#include <string>
#include <vector>

class NodeRequestGenerator {
   public:
    NodeRequestGenerator(const std::vector<std::string>& names_)
        : names(names_) {}

    ~NodeRequestGenerator() {
        for (auto& i : children) {
            delete i;
        }
    }

    unsigned int getMass() const;

    std::vector<std::string> names;
    std::vector<NodeRequestGenerator*> children;
};

class TreeRequest {
   public:
    ~TreeRequest() { delete head; }

    unsigned int getMass() const {
        return (head == nullptr) ? (0) : (head->getMass());
    }

    NodeRequestGenerator* head = nullptr;
};

class RequestGenerator {
   public:
    RequestGenerator(const std::string& file);
    ~RequestGenerator();

   private:
    void createTree(const std::string& file);

    TreeRequest tree;
};

#endif  // OURSQL_REQUESTGENERATOR_H
