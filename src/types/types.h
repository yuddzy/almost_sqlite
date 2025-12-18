#pragma once

#define _CRT_SECURE_NO_WARNINGS

// Отключаем ВСЕ возможные макросы Windows
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <cstdint>
#include <vector>

// Типы данных
enum All_types {
    BIT = 0,
    TINYINT = 1,
    SMALLINT = 2,
    INT_TYPE = 3,      // Используем INT_TYPE вместо INT_TYPE
    BIGINT = 4,
    FLOAT_TYPE = 5,    // Используем FLOAT_TYPE вместо FLOAT_TYPE
    REAL = 6,
    DATETIME = 7,
    SMALLDATETIME = 8,
    DATE_TYPE = 9,     // Используем DATE_TYPE вместо DATE_TYPE
    TIME_TYPE = 10,    // Используем TIME_TYPE вместо TIME_TYPE
    CHAR_TYPE = 11,    // Используем CHAR_TYPE вместо CHAR_TYPE
    VARCHAR = 12,
    TEXT_TYPE = 13
};

// Структура для столбца
struct Column {
    std::string name;  // Используем std::string
    All_types type;
    int size;

    // Конструктор с параметрами - правильный синтаксис
    Column(const std::string& n, All_types t, int s = 0)
        : name(n), type(t), size(s) {
    }
};

// Структура для связанного списка типов данных
struct data_list_node {
    All_types type;
    int size;
    struct data_list_node* next;
};

typedef struct data_list_node data_node;

// Объявления функций
data_node* init(All_types type, int size);
data_node* append(data_node* root, All_types type, int size);
void destroy(data_node* root);
int get_type_size(All_types column, int size = -1);
All_types get_type_from_string(const std::string& type_str);
int get_varchar_size(const std::string& type_str);