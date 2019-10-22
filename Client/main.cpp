#include <iostream>

#include "Client.h"

int main() {
    Client client("localhost", 11234);
    client.connect();
    while (1) {
        std::string s;
        std::string ans;
        std::getline(std::cin, s);
        auto exception = client.request(s, ans);
        std::cout << "Exception: " << exception << "\nMessage:\n" << ans;
    }
}
