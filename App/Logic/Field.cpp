
#include "Field.h"

void Field::getVal(void *dst) {

    switch (type_) {
        case Types::INT_:
            *(static_cast<int *>(dst)) = iVal_;
            break;
        case Types::REAL_:
            *(static_cast<double *>(dst)) = dVal_;
            break;
/*        case Types::TEXT_:
            *(static_cast<char **>(dst)) = tVal_;
            break;*/
    }

}
