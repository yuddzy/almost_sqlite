#ifndef TYPEERROR_H
#define TYPEERROR_H

#include <string>
#include <stdexcept>

using namespace std;

// Коды ошибок
enum ErrorCode {
    INVALID_SYNTAX = 1,
    TABLE_ALREADY_EXISTS = 2,
    TABLE_NOT_EXISTS = 3,
    COLUMN_NOT_FOUND = 4,
    TYPE_MISMATCH = 5,
    DUPLICATE_COLUMN = 6
};

// Класс ошибки
class typeError : public runtime_error {
public:
    ErrorCode code;

    typeError(ErrorCode errorCode, const string& message)
        : runtime_error(message), code(errorCode) {
    }
};

#endif // TYPEERROR_H