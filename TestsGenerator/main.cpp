#include <fstream>
#include <iostream>

#include "parser.cpp"

const std::string file = "../../Tests/AutoGenTests.cpp";

unsigned int skipSpaces(const std::string& s, unsigned int from = 0) {
    for (from; from < s.size(); from++) {
        if (s[from] != ' ') {
            return from;
        }
    }
    return s.size();
}

inline bool isEngAlpha(char a) {
    a = tolower(a);
    return 'a' <= a && a <= 'z';
}

int getName(const std::string& s, std::string& ans, unsigned int from = 0) {
    unsigned int start = from = skipSpaces(s, from);
    if (from >= s.size() || !isEngAlpha(s[from])) {
        return -1;
    }
    for (from;
         from < s.size() && (isEngAlpha(s[from]) || std::isdigit(s[from]));
         from++) {
    }
    ans = s.substr(start, from - start);
    return from;
}

void createTestsFile() {
    std::ofstream f_out(file);
    f_out << "#include <gmock/gmock.h>\n"
             "#include <gtest/gtest.h>\n"
             "\n"
             "#include \"parser.cpp\""
             "\n\n";
    f_out.close();
}

unsigned int countTest(std::ifstream& f) {
    unsigned int count = 0;
    std::string s;
    while (std::getline(f, s)) {
        if (s.size() > 4 && (strncmp(s.c_str(), "TEST", 4) == 0)) {
            count++;
        }
    }
    return count;
}

void createTest(const std::string& class_test, unsigned int id,
                const std::string& request, const std::string& answer) {
    std::ofstream f(file, std::ios::app);
    f << "TEST(" << class_test << ", TEST_" << id << ") {\n"
      << "    parse_string(\"" << request << "\");\n"
      << "    //  TODO: фигня для провекри ответа\n"
      << "}\n\n";
    f.close();
}

int main() {
    std::string request;
    std::string answer;
    std::string s;

    std::ifstream f_in(file);
    if (!f_in.is_open()) {
        createTestsFile();
        f_in = std::ifstream(file);
    }
    unsigned int numberTest = countTest(f_in);
    f_in.close();
    std::getline(std::cin, s);
    while (s != "~e") {
        if (s.size() >= 2 && s[0] == '~' && s[1] == 's') {
            std::string name;
            if (getName(s, name, 2) < 0) {
                name = "AGT";
            }
            createTest(name, ++numberTest, request, answer);
            std::cout << "Test " << name << " " << numberTest << " created."
                      << std::endl;
        } else {
            request = s;
            parse_string(request.c_str());
            // TODO: read answer
        }
        std::getline(std::cin, s);
    }
    return 0;
}
