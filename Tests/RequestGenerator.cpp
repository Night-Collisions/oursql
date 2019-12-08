#include "RequestGenerator.h"

#include <iostream>
#include <fstream>

unsigned int NodeRequestGenerator::getMass() const {
    unsigned int children_mass = 0;
    for (const auto &i : children) {
        children_mass += i->getMass();
    }
    return std::max(1U, std::max(1U, names.size()) * children_mass);
}

RequestGenerator::RequestGenerator(const std::string& file) {
    createTree(file);
}

RequestGenerator::~RequestGenerator() {
    delete tree.head;
}

void addNodeChildrens(NodeRequestGenerator* node, std::ifstream& f);

std::vector<std::string> name2names(const std::string &s) {
    std::vector<std::string> ans(1);
    for (auto &i: s) {
        if (i == ' ' || i == '\n') {
            continue;
        }
        if (i == '|') {
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
    if (c == '/' || c == EOF) {
        return nullptr;
    }
    std::string name;
    while (c != '\\' && c != EOF) {
        name.push_back(c);
        f >> c;
    }
    NodeRequestGenerator* new_node = new NodeRequestGenerator(name2names(name));
    addNodeChildrens(new_node, f);
    return new_node;
}

void addNodeChildrens(NodeRequestGenerator* node, std::ifstream& f) {
    NodeRequestGenerator* children;
    children = getNode(f);
    while (children != nullptr) {
        node->children.push_back(children);
        children = getNode(f);
    }
}

void RequestGenerator::createTree(const std::string& file) {
    std::ifstream f(file);
    if (!f.is_open()) {
        std::cerr << "File doesn't exist!" << std::endl;
        return;
    }

    delete tree.head;
    tree.head = getNode(f);
    f.close();
}

