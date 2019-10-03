#ifndef OURSQL_APP_EXCEPTIONS_QUERYEXCEPTION_H_
#define OURSQL_APP_EXCEPTIONS_QUERYEXCEPTION_H_

#include <exception>
#include <string>

class QueryException : public std::exception {
   public:
    explicit QueryException(const std::string& message)
        : message_(message) {}

    [[nodiscard]] const char* what() const noexcept;

   private:
    std::string message_;
};

#endif  // OURSQL_APP_EXCEPTIONS_QUERYEXCEPTION_H_
