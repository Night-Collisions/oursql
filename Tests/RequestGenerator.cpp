#include "RequestGenerator.h"

#include <c++/functional>
#include <fstream>
#include <iostream>

#define ARGUMENTS_SPLIT ':'

RequestGenerator::RequestGenerator(const std::string& file) {
    createTree(file);
}

RequestGenerator::~RequestGenerator() {
    //    delete tree.head;
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
        node = getNode(f);
        if (name.front() == '+' || name.front() == '-') {
            trees_.insert({name, node});
        } else {
            std::cerr << "The file is damaged!" << std::endl;
            return;
        }
    }
    f.close();

    max_mass_ = 0;
    auto it = trees_.begin();
    while (it != trees_.end()) {
        if (it->first.size() > 0 && it->first.front() == '+') {
            max_mass_ += it->second->getMass();
        }
        it++;
    }
}

std::string RequestGenerator::getRecuest() {
    unsigned int numb = rand() % max_mass_;
    unsigned int sum = 0;
    NodeRequestGenerator* node;
    auto it = trees_.begin();
    while (it != trees_.end()) {
        if (it->first.size() > 0 && it->first.front() == '+') {
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
        if (s.front() == '~') {
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
        } else if (s.front() == '-' || s.front() == '+') {
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
    std::string command = names[rand() % names.size()];
    ans += implCommand(command);
    auto children = node->getChildren();
    if (!children.empty()) {
        std::string s = createRequest(children[rand() % children.size()]);
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
                       (rand() % 26) + (((rand() % 2) == 1) ? ('A') : ('a')));
}

std::string RequestGenerator::getFigure(const std::string& s) {
    return std::string(1, rand() % 10 + '0');
}

std::string RequestGenerator::getWord(const std::string& s) {
    std::string ans = getLater();
    while ((rand() % 5) != 0) {
        ans.append(getLater());
    }
    return ans;
}

std::string RequestGenerator::getIntNumber(const std::string& s) {
    std::string ans = (((rand() % 2) == 0) ? ("-") : ("")) + getFigure();
    while ((rand() % 4) != 0) {
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
        &RequestGenerator::getField};
    size_t opened_gunnel = 0;
    std::string ans;
    while ((rand() % 5) != 0) {
        if ((rand() % 3) != 0) {
            ans += "(";
            opened_gunnel++;
        }
        ans += (this->*(fun[rand() % nuber_kind_operants]))("");
        if ((opened_gunnel > 0) && (rand() % 3) == 0) {
            ans += ")";
            opened_gunnel--;
        }
        ans += " " + signs[rand() % signs.size()] + " ";
    }
    ans += (this->*(fun[rand() % nuber_kind_operants]))("");
    while (opened_gunnel > 0) {
        opened_gunnel--;
        ans += ")";
    }
    return ans;
}

std::string RequestGenerator::getBoolExpression(const std::string& s) {
    std::array<std::string, 6> sign = {"=", "<", ">", "<=", ">=", "!="};
    std::string ans;
    size_t opened_gunnel = 0;
    do {
        if ((rand() % 3) != 0) {
            ans += "(";
            opened_gunnel++;
        }
        ans += getMathExpression(s);
        if ((opened_gunnel > 0) && (rand() % 3) == 0) {
            ans += ")";
            opened_gunnel--;
        }
        ans += " " + sign[rand() % sign.size()] + " ";
    } while ((rand() % 2) != 0);
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
    while ((rand() % 4) != 0) {
        ans += split + " " + implCommand(command);
    }
    return ans;
}

std::string RequestGenerator::getField(const std::string& s) {
    std::string ans = getTable();
    while ((rand() % 3) == 0) {
        ans += std::string(".") + getTable();
    }
    return ans;
}

std::string RequestGenerator::getTable(const std::string& s) {
    std::string ans = getLater();
    while ((rand() % 3) != 0) {
        ans += ((rand() % 3) == 0) ? (getFigure()) : (getLater());
    }
    return ans;
}