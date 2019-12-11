#include "RequestGenerator.h"

#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

#define ARGUMENTS_SPLIT ':'
#define STANDARD_FUNCTION '~'
#define START_NON_GENERATED '-'
#define START_GENERATED '+'

RequestGenerator::RequestGenerator(const std::string& file) {
    createTree(file);
    srand(time(0));
}

RequestGenerator::~RequestGenerator() {
    auto it = trees_.begin();
    while (it != trees_.end()) {
        delete it->second;
        it++;
    }
}

void addNodeChildrens(NodeRequestGenerator* node, std::ifstream& f);

std::vector<std::string> name2names(const std::string& s) {
    std::vector<std::string> ans(1);
    for (auto& i : s) {
        if (i == '\n') {
            continue;
        }
        if (i == '|') {
            while (ans.back().size() > 0 &&
                   (ans.back().back() == ' ' || ans.back().back() == '\n')) {
                ans.back().pop_back();
            }
            ans.emplace_back();
        } else {
            ans.back().push_back(i);
        }
    }
    return ans;
}

NodeRequestGenerator* getNode(std::ifstream& f) {
    char c;
    while (!(f >> c) && c != ' ') {
    }
    if (c == '/') {
        return nullptr;
    }
    std::string name;
    int c_int = c;
    while (c_int != '\\' && c_int != EOF) {
        name.push_back(c_int);
        c_int = f.get();
    }
    NodeRequestGenerator* new_node = new NodeRequestGenerator(name2names(name));
    addNodeChildrens(new_node, f);
    return new_node;
}

void addNodeChildrens(NodeRequestGenerator* node, std::ifstream& f) {
    NodeRequestGenerator* children;
    children = getNode(f);
    while (children != nullptr) {
        node->addChild(children);
        children = getNode(f);
    }
}

void RequestGenerator::createTree(const std::string& file) {
    std::ifstream f(file);
    if (!f.is_open()) {
        std::cerr << "File doesn't exist!" << std::endl;
        return;
    }

    std::string name;
    NodeRequestGenerator* node;
    while (f >> name) {
        if (name.front() == START_GENERATED || name.front() == START_NON_GENERATED) {
            if (name.size() > 3 && name[0] == START_NON_GENERATED && name[1] == STANDARD_FUNCTION) {
                if (name.size() < 3 || name.back() != '/' ||
                    name[name.size() - 2] != '\\') {
                    int c;
                    while ((c = f.get()) != EOF &&
                        (name.size() < 3 || (name.back() != '/' &&
                            name[name.size() - 2] != '\\'))) {
                        name.push_back(c);
                    }
                }
                implCommand(name.substr(1, name.size() - 1));
            } else {
                node = getNode(f);
                trees_.insert({name, node});
            }
        } else {
            std::cerr << "The file is damaged!" << std::endl;
            return;
        }
    }
    f.close();

    max_mass_ = 0;
    auto it = trees_.begin();
    while (it != trees_.end()) {
        if (it->first.size() > 0 && it->first.front() == START_GENERATED) {
            max_mass_ += it->second->getMass();
        }
        it++;
    }
}

std::string RequestGenerator::getRequest() {
    unsigned int numb = Rand() % max_mass_;
    unsigned int sum = 0;
    NodeRequestGenerator* node;
    auto it = trees_.begin();
    while (it != trees_.end()) {
        if (it->first.size() > 0 && it->first.front() == START_GENERATED) {
            sum += it->second->getMass();
            if (sum >= numb) {
                node = it->second;
                break;
            }
        }
        it++;
    }
    return createRequest(node) + ';';
}

std::string RequestGenerator::implCommand(const std::string& s) {
    if (s.size() > 0) {
        if (s.front() == STANDARD_FUNCTION) {
            unsigned int param = 0;
            for (param = 0; param < s.size() && s[param] != ARGUMENTS_SPLIT;
                 param++) {
            }
            auto substr = s.substr(0, param);
            auto it = comands_.find(substr);
            if (it == comands_.end()) {
                std::cerr << s << ":: is wrong command!" << std::endl;
                return "";
            }
            std::string args = (++param < s.size()) ? (s.substr(param)) : "";
            return (this->*(it->second))(args);
        } else if (s.front() == START_NON_GENERATED || s.front() == START_GENERATED) {
            auto it = trees_.find(s);
            if (it == trees_.end()) {
                std::cerr << s << ":: is wrong command!" << std::endl;
                return "";
            }
            return createRequest(it->second);
        } else {
            return s;
        }
    }
    return "";
}

std::string RequestGenerator::createRequest(NodeRequestGenerator* node) {
    std::string ans;
    auto names = node->getNames();
    std::string command = names[Rand() % names.size()];
    ans += implCommand(command);
    auto children = node->getChildren();
    if (!children.empty()) {
        std::string s = createRequest(children[Rand() % children.size()]);
        if (ans.empty()) {
            ans = s;
        } else if (!s.empty()) {
            ans += " " + s;
        }
    }
    return ans;
}

std::string RequestGenerator::getLater(const std::string& s) {
    return std::string(1,
                       (Rand() % 26) + (((Rand() % 2) == 1) ? ('A') : ('a')));
}

std::string RequestGenerator::getFigure(const std::string& s) {
    return std::string(1, Rand() % 10 + '0');
}

std::string RequestGenerator::getWord(const std::string& s) {
    std::string ans = getLater();
    while ((Rand() % 5) != 0) {
        ans.append(getLater());
    }
    return ans;
}

std::string RequestGenerator::getIntNumber(const std::string& s) {
    std::string ans = (((Rand() % 2) == 0) ? ("-") : ("")) + getFigure();
    while ((Rand() % 4) != 0) {
        ans += getFigure();
    }
    return ans;
}

std::string RequestGenerator::getFloatNumber(const std::string& s) {
    auto n = getIntNumber();
    return getIntNumber() + "." + ((n.size() > 1) ? n.substr(1) : n);
}

std::string RequestGenerator::getMathExpression(const std::string& s) {
    size_t nuber_kind_operants = 2 + ((s == "field") ? (1) : (0));
    std::array<std::string, 4> signs = {"+", "/", "-", "*"};
    std::array<std::string (RequestGenerator::*)(const std::string&), 3> fun = {
        &RequestGenerator::getFloatNumber, &RequestGenerator::getFigure,
        &RequestGenerator::getName};
    size_t opened_gunnel = 0;
    std::string ans;
    while ((Rand() % 5) != 0) {
        if ((Rand() % 3) != 0) {
            ans += "(";
            opened_gunnel++;
        }
        ans += (this->*(fun[Rand() % nuber_kind_operants]))("");
        if ((opened_gunnel > 0) && (Rand() % 3) == 0) {
            ans += ")";
            opened_gunnel--;
        }
        ans += " " + signs[Rand() % signs.size()] + " ";
    }
    ans += (this->*(fun[Rand() % nuber_kind_operants]))("");
    while (opened_gunnel > 0) {
        opened_gunnel--;
        ans += ")";
    }
    return ans;
}

std::string RequestGenerator::getBoolOperator(const std::string& s) {
    std::array<std::string, 6> sign = {"=", "<", ">", "<=", ">=", "!="};
    return sign[Rand() % sign.size()];
}

std::string RequestGenerator::getBoolExpression(const std::string& s) {
    std::string ans;
    size_t opened_gunnel = 0;
    do {
        if ((Rand() % 3) != 0) {
            ans += "(";
            opened_gunnel++;
        }
        ans += getMathExpression(s);
        if ((opened_gunnel > 0) && (Rand() % 3) == 0) {
            ans += ")";
            opened_gunnel--;
        }
        ans += " " + getBoolOperator() + " ";
    } while ((Rand() % 2) != 0);
    ans += getMathExpression(s);
    while (opened_gunnel > 0) {
        opened_gunnel--;
        ans += ")";
    }
    return ans;
}

std::string RequestGenerator::getArrayOf(const std::string& s) {
    std::string command;
    unsigned int i;
    for (i = 0; i < s.size() && s[i] != ARGUMENTS_SPLIT;
         command.push_back(s[i++])) {
    }
    std::string split = (++i < s.size()) ? (s.substr(i)) : "";
    std::string ans = implCommand(command);
    while ((Rand() % 4) != 0) {
        ans += split + " " + implCommand(command);
    }
    return ans;
}

std::string RequestGenerator::getNested(const std::string& s) {
    std::string ans = getName();
    while ((Rand() % 3) == 0) {
        ans += std::string(".") + getNested();
    }
    return ans;
}

std::string to_lower(const std::string &s) {
    auto ans = s;
    for (auto &i : ans) {
        i = tolower(i);
    }
    return ans;
}

std::string RequestGenerator::getName(const std::string& s) {
    std::string ans = getLater();
    while (((Rand() % 3) != 0) ||
           (not_generated_names_.find(to_lower(ans)) != not_generated_names_.end())) {
        ans += ((Rand() % 3) == 0) ? (getFigure()) : (getLater());
    }
    return ans;
}

std::string RequestGenerator::setNotGeneratedNames(const std::string& s) {
    std::stringstream ss(s);
    std::string word;
    while (ss >> word) {
        if (word.back() == '/') {
            word.pop_back();
            word.pop_back();
            if (word.empty()) {
                return "";
            }
        }
        not_generated_names_.insert(word);
    }
    return "";
}