#ifndef TEST_TYPES_H  
#define TEST_TYPES_H  
#include "types.h"
#include "test_types.h"

// Declare the test_data function  
void test_data(All_types type, const char* input, bool not_null, int max_length);

#endif // TEST_TYPES_H

int main_main5() {

    // BIT
    test_data(BIT, "1");
    test_data(BIT, "2");

    // TINYINT
    test_data(TINYINT, "100");
    test_data(TINYINT, "300");

    // SMALLINT
    test_data(SMALLINT, "25000");
    test_data(SMALLINT, "40000");

    // INT_TYPE
    test_data(INT_TYPE, "2147483647");
    test_data(INT_TYPE, "3000000000");

    // BIGINT
    test_data(BIGINT, "9223372036854775807");
    test_data(BIGINT, "9999999999999999999");

    // FLOAT_TYPE
    test_data(FLOAT_TYPE, "3.141592");
    test_data(FLOAT_TYPE, "3.14159265");

    // REAL
    test_data(REAL, "3.141592653589793");
    test_data(REAL, "3.1415926535897932");

    // DATE_TYPE
    test_data(DATE_TYPE, "2023-12-25");
    test_data(DATE_TYPE, "2023-13-01");

    // TIME_TYPE
    test_data(TIME_TYPE, "14:30:00.500");
    test_data(TIME_TYPE, "25:00:00");

    // DATETIME
    test_data(DATETIME, "2023-12-25 14:30:00.123456");
    test_data(DATETIME, "2023-12-25 14:30:00.1234567");

    // CHAR_TYPE
    test_data(CHAR_TYPE, "Hello", true, 10);
    test_data(CHAR_TYPE, "VeryLongString", true, 5);

    // VARCHAR
    test_data(VARCHAR, "Testirov", true, 8);
    test_data(VARCHAR, "TooLongText", true, 5);

    // TEXT_TYPE
    test_data(TEXT_TYPE, "This is a long text");
    test_data(TEXT_TYPE, "");

    return 0;
}