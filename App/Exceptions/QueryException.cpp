#include "QueryException.h"

const char *QueryException::what() const noexcept
{
    return message_.c_str();
}