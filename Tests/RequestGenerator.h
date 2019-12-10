#ifndef OURSQL_REQUESTGENERATOR_H
#define OURSQL_REQUESTGENERATOR_H

#include <map>
#include <string>
#include <vector>

class NodeRequestGenerator {
   public:
    NodeRequestGenerator(const std::vector<std::string>& names)
        : names_(names) {}

    ~NodeRequestGenerator() {
        for (auto& i : children_) {
            delete i;
        }
    }

    unsigned int getMass() const { return std::max(1U, mass_); }

    unsigned int getChildrenMass() const {
        unsigned int ans = 0;
        for (auto &i : children_) {
            ans += i->getMass();
        }
        return ans;
    }

    std::vector<std::string> getNames() { return names_; }
    void addName(const std::string& s) {
        names_.push_back(s);
        if (names_.size() > 1) {
            mass_ = mass_ / (names_.size() - 1) * names_.size();
        }
    }

    std::vector<NodeRequestGenerator*> getChildren() { return children_; }
    void addChild(NodeRequestGenerator* c) {
        children_.push_back(c);
        mass_ += c->getMass() * names_.size();
    }

   protected:
    std::vector<std::string> names_;
    std::vector<NodeRequestGenerator*> children_;
    unsigned int mass_ = 0;
};

class RequestGenerator {
   public:
    RequestGenerator(const std::string& file);
    ~RequestGenerator();

    std::string getRecuest();

   private:
    void createTree(const std::string& file);

    std::string createRequest(NodeRequestGenerator *node);
    std::string implCommand(std::string &);

    static std::string getChar(const std::string &s = "") {}
    static std::string getWord(const std::string &s = "") {}
    static std::string getIntNumber(const std::string &s = "") {}
    static std::string getFloatNumber(const std::string &s = "") {}
    static std::string getMathExpression(const std::string &s = "") {}
    static std::string getBoolExpression(const std::string &s = "") {}
    static std::string getArrayOf(const std::string &s) {}
    static std::string getField(const std::string &s = "") {}
    static std::string getTable(const std::string &s = "") {}
    static std::string getMathExpressionWithField(const std::string &s = "") {}
    static std::string getBoolExpressionWithField(const std::string &s = "") {}

    std::map<std::string, NodeRequestGenerator*> trees_;
    const std::map<std::string, std::string (*)(const std::string &)> comands_ = {
        {"~char", getChar},
        {"~word", getWord},
        {"~int_number", getIntNumber},
        {"~float_number", getFloatNumber},
        {"~math_expr", getMathExpression},
        {"~bool_expr", getBoolExpression},
        {"~array_of", getArrayOf},
        {"~field", getField},
        {"~table", getTable},
        {"~math_expr_with_field", getMathExpressionWithField},
        {"~bool_expr_with_field", getBoolExpressionWithField}
    };
    unsigned int max_mass_ = 0;
};

#endif  // OURSQL_REQUESTGENERATOR_H
