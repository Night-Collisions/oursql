#ifndef OURSQL_OUR_H
#define OURSQL_OUR_H

#include <iostream>

namespace ourSQL {
extern volatile unsigned long long transaction_number_;

unsigned int perform(std::istream& in = std::cin, std::ostream& out = std::cout,
                     unsigned short client_id = 0);
void nonstop_perform(std::istream& in = std::cin, std::ostream& out = std::cout,
                     unsigned short client_id = 0);

void forget_client(unsigned short client_id);
}  // namespace ourSQL

#endif  // OURSQL_OUR_H
