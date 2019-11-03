#include <iostream>

#include "Server.h"

int main() {
    ourSQL::server::run_server(11234);
    return 0;
}
