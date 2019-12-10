#include "RequestGenerator.h"

#include <fstream>
#include <iostream>

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

    // TODO: delete

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
    }
}

std::string RequestGenerator::getRecuest() {
    unsigned int numb = rand() % max_mass_;
    unsigned int sum = 0;
    NodeRequestGenerator *node;
    auto it = trees_.begin();
    while (it != trees_.end()) {
        if (it->first.size() > 0 && it->first.front() == '+') {
            sum += it->second->getMass();
            if (sum >= numb) {
                node = it->second;
                break;
            }
        }
    }
    return createRequest(node) + ';';
}

std::string RequestGenerator::implCommand(std::string &s) {
    if (s.size() > 0) {
        if (s.front() == '~') {
            auto it = comands_.find(s);
            if (it == comands_.end()) {
                std::cerr << s << ":: is wrong command!" << std::endl;
                return "";
            }
            //TODO:: вырезать подстроку
            return it->second();
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

std::string RequestGenerator::createRequest(NodeRequestGenerator *node) {
    std::string ans;
    auto names = node->getNames();
    std::string command = names[rand() % names.size()];
    ans += " " + implCommand(command);
    //TODO: пройтись по детям
}
