#ifndef OURSQL_REQUESTGENERATOR_H
#define OURSQL_REQUESTGENERATOR_H

#include <map>
#include <string>
#include <unordered_set>
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
        for (auto& i : children_) {
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

    std::string getRequest();

   private:
    void createTree(const std::string& file);

    std::string createRequest(NodeRequestGenerator* node);
    std::string implCommand(const std::string&);

    unsigned int Rand() { return rand(); }

    std::string getLater(const std::string& s = "");
    std::string getFigure(const std::string& s = "");
    std::string getWord(const std::string& s = "");
    std::string getIntNumber(const std::string& s = "");
    std::string getFloatNumber(const std::string& s = "");
    std::string getMathExpression(const std::string& s = "");
    std::string getBoolOperator(const std::string& s = "");
    std::string getBoolExpression(const std::string& s = "");
    std::string getArrayOf(const std::string& s);
    std::string getName(const std::string& s = "");
    std::string getNested(const std::string& s = "");
    std::string setNotGeneratedNames(const std::string& s);

    std::map<std::string, NodeRequestGenerator*> trees_;
    const std::map<std::string,
                   std::string (RequestGenerator::*)(const std::string&)>
        comands_ = {{"~later", &RequestGenerator::getLater},
                    {"~figure", &RequestGenerator::getFigure},
                    {"~word", &RequestGenerator::getWord},
                    {"~int_number", &RequestGenerator::getIntNumber},
                    {"~float_number", &RequestGenerator::getFloatNumber},
                    {"~math_expr", &RequestGenerator::getMathExpression},
                    {"~bool_operator", &RequestGenerator::getBoolOperator},
                    {"~bool_expr", &RequestGenerator::getBoolExpression},
                    {"~array_of", &RequestGenerator::getArrayOf},
                    {"~name", &RequestGenerator::getName},
                    {"~nested", &RequestGenerator::getNested},
                    {"~set_not_generated_names",
                     &RequestGenerator::setNotGeneratedNames}};
    std::unordered_set<std::string> not_generated_names_;
    unsigned int max_mass_ = 0;
};

#endif  // OURSQL_REQUESTGENERATOR_H