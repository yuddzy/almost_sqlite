#pragma once
#ifndef CONDITION_H
#define CONDITION_H

#include "types/types.h"
#include <cstring>
#include <string>

// Операторы сравнения для WHERE условий
enum Operator {
    EQUAL,              // =
    NOT_EQUAL,          // !=
    LESS,               // <
    LESS_OR_EQUAL,      // <=
    GREATER,            // >
    GREATER_OR_EQUAL,   // >=
    LIKE,               // LIKE
    IN_OP,               // IN
    BETWEEN
};

// Структура для представления условия WHERE
struct Condition {
    char column_name[64];    // Имя колонки
    unsigned int operator_id; // ID оператора
    All_types type;          // Тип данных для сравнения
    void* compare;           // Указатель на значение для сравнения

    // Конструктор по умолчанию
    Condition() : operator_id(0), type(All_types::INT_TYPE), compare(nullptr) {
        memset(column_name, 0, sizeof(column_name));
    }

    // Конструктор с параметрами
    Condition(const std::string& col, Operator op, All_types t, void* val)
        : operator_id(static_cast<unsigned int>(op)), type(t), compare(val) {
        strncpy(column_name, col.c_str(), sizeof(column_name) - 1);
        column_name[sizeof(column_name) - 1] = '\0';
    }

    // Конструктор копирования
    Condition(const Condition& other) {
        strncpy(column_name, other.column_name, sizeof(column_name));
        operator_id = other.operator_id;
        type = other.type;
        compare = nullptr;

        if (other.compare != nullptr) {
            // Копируем память в зависимости от типа
            int size = get_type_size(other.type, 0);
            if (size > 0) {
                compare = malloc(size);
                memcpy(compare, other.compare, size);
            }
        }
    }

    // Оператор присваивания
    Condition& operator=(const Condition& other) {
        if (this != &other) {
            // Освобождаем старую память
            if (compare != nullptr) {
                free(compare);
            }

            strncpy(column_name, other.column_name, sizeof(column_name));
            operator_id = other.operator_id;
            type = other.type;
            compare = nullptr;

            if (other.compare != nullptr) {
                int size = get_type_size(other.type, 0);
                if (size > 0) {
                    compare = malloc(size);
                    memcpy(compare, other.compare, size);
                }
            }
        }
        return *this;
    }

    // Деструктор
    ~Condition() {
        if (compare != nullptr) {
            free(compare);
            compare = nullptr;
        }
    }
};

// Вспомогательная функция для создания Condition
inline Condition create_condition(const std::string& col_name, Operator op,
    All_types value_type, void* comp) {
    Condition cond;
    memset(cond.column_name, 0, sizeof(cond.column_name));
    strncpy(cond.column_name, col_name.c_str(), sizeof(cond.column_name) - 1);
    cond.operator_id = static_cast<unsigned int>(op);
    cond.type = value_type;
    cond.compare = comp;
    return cond;
}

#endif // CONDITION_H