#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <sstream>

#include "../../App/Our.h"

inline bool isEngAlpha(char a) {
    a = tolower(a);
    return 'a' <= a && a <= 'z';
}

bool isName(const std::string& s) {
    if(!isEngAlpha(s[0])) {
        return false;
    }
    for (unsigned int i = 1; i < s.size(); i++) {
        if (!isEngAlpha(s[i]) && !std::isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

std::string toText(const std::string& s) {
    std::string ans;
    for (const auto& i : s) {
        if (i == '\\') {
            ans.push_back('\\');
        }
        ans.push_back(i);
    }
    return ans;
}

class TG {
   public:
    TG() {
        std::ifstream f(file_);
        if (!f.is_open()) {
            std::ofstream f_out(file_);
            f_out << "#include <gmock/gmock.h>\n"
                     "#include <gtest/gtest.h>\n"
                     "\n"
                     "#include \"Test.h\"\n"
                     "#include \"../../App/Our.h\"\n"
                     "\n\n" << std::endl;
            f_out.close();
        }
        f = std::ifstream(file_);
        while (!f.eof()) {
            std::string s;
            std::getline(f, s);
            addTestNameFromFile(s);
        }
    }

    void perform_command() {
        while (1) {
            std::string s;
            do {
                s.push_back(std::cin.get());
                if (s.back() == '\n') {
                    s.pop_back();
                }
            } while (s.back() != ';' && s.back() != '~');

            if (s.back() == '~' && std::cin.get() == 's') {
                std::string group = " ";
                for (; !isEngAlpha(group.back()) && !isdigit(group.back()); group.back() = std::cin.get()) {}
                for (; group.back() != ' '; group.push_back(tolower(std::cin.get()))) {}
                group.pop_back();
                std::string name = " ";
                for (; !isEngAlpha(name.back()) && !isdigit(name.back()); name.back() = std::cin.get()) {}
                for (; name.back() != ' ' &&  name.back() != '\n'; name.push_back(tolower(std::cin.get()))) {}
                name.pop_back();
                addTest(group, name);
                commands_.clear();
            } else {
                std::stringstream in;
                for (int i = 0; i < s.size(); i++) {
                    in << s[i];
                    std::cout << int(s[i]) << std::endl;
                }
                std::stringstream out;
                commands_.push_back({s, ""});
                ourSQL::perform(in, out);
                commands_.back().second = out.str();
                std::cout << commands_.back().second;
            }
        }
    }

   private:
    bool addTest(const std::string& group, const std::string& name) {
        if (!isName(group)) {
            std::cerr << "Wrong name group!" << std::endl;
            return false;
        }
        if (!isName(name)) {
            std::cerr << "Wrong test name!" << std::endl;
            return false;
        }
        const auto test_name = group + '~' + name;
        if (tests_.find(test_name) != tests_.end()) {
            return false;
        }
        tests_.insert(test_name);

        std::ofstream f(file_, std::ios::app);
        f << "TEST(" << group << ", " << name << ") {" << std::endl;
        for (auto& i : commands_) {
            f << "    std::stringstream in(" + i.first + ");\n"
              << "    std::stringstream out;\n"
              << "    ourSQL::perform(in, out);\n"
              << "    (" << toText(i.second) << ", out.str());" << std::endl;
        }
        f << "}" << std::endl;
        return true;
    }

    void addTestNameFromFile(std::string& s) {
        std::string name;
        const std::string start_test = "TEST(";
        if (s.size() > start_test.size() && s.compare(0, start_test.size(), start_test) == 0) {
            unsigned int i = start_test.size();
            for (i; i < s.size() && s[i] != ','; i++) {
                name.push_back(tolower(s[i]));
            }
            for (++i; i < s.size() && s[i] == ' '; i++) {
            }
            name.push_back('~');
            for (i; i < s.size() && s[i] != ')'; i++) {
                name.push_back(tolower(s[i]));
            }
            tests_.insert(name);
        }
    }

    std::vector<std::pair<std::string, std::string>> commands_;
    std::string group_name_;
    std::set<std::string> tests_;
    const std::string file_ = "../../../Testing/Tests/AutoGenTests.cpp";
};

int main() {
    TG tg;
    tg.perform_command();
    return 0;
}
