#define _CRT_SECURE_NO_WARNINGS
#include "config/config.h"
#include "Command.h"
#include "db.h"
#include <iostream>
#include <cstring>

using namespace std;

// ================= CreateCommand =================

CreateCommand::CreateCommand(const string& table_name,
    const vector<string>& column_names,
    const vector<string>& data_types,
    const vector<bool>& is_nullable)
    : table_name_(table_name), column_names_(column_names),
    data_types_(data_types), is_nullable_(is_nullable) {
}

void CreateCommand::execute(SimpleDB* db) {
    if (!validate()) {
        throw typeError(INVALID_SYNTAX, "Ошибка валидации CREATE TABLE команды");
    }

    cout << "Создаем таблицу: " << table_name_ << endl;
    cout << "Колонки:" << endl;

    for (size_t i = 0; i < column_names_.size(); i++) {
        cout << "  " << column_names_[i] << " " << data_types_[i];
        if (!is_nullable_[i]) {
            cout << " NOT NULL";
        }
        cout << endl;
    }

    db->createTable(table_name_);
    cout << "✓ Таблица '" << table_name_ << "' создана" << endl;
}

string CreateCommand::getCommandType() const {
    return "CREATE TABLE";
}

bool CreateCommand::validate() {
    if (table_name_.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }
    if (column_names_.size() != data_types_.size() ||
        column_names_.size() != is_nullable_.size()) {
        throw typeError(INVALID_SYNTAX, "Количество имен колонок, типов данных и флагов nullable не совпадает");
    }
    if (column_names_.empty()) {
        throw typeError(INVALID_SYNTAX, "Таблица должна содержать хотя бы одну колонку");
    }

    for (size_t i = 0; i < column_names_.size(); i++) {
        if (column_names_[i].empty()) {
            throw typeError(INVALID_SYNTAX, "Имя колонки не может быть пустым");
        }
    }

    return true;
}

// ================= SelectCommand =================

SelectCommand::SelectCommand(const vector<string>& columns,
    const string& table_name,
    const vector<Condition>& where_conditions)
    : columns_(columns), table_name_(table_name), where_conditions_(where_conditions) {
    // Временно закомментированы
    // order_by_(order_by), limit_(limit) {
}

void SelectCommand::execute(SimpleDB* db) {
    if (!validate()) {
        throw typeError(INVALID_SYNTAX, "Ошибка валидации SELECT команды");
    }

    // Проверяем существование таблицы
    if (!db->tableExists(table_name_)) {
        throw typeError(TABLE_NOT_EXISTS, "Таблица '" + table_name_ + "' не существует");
    }

    cout << "Выполняем SELECT из таблицы: " << table_name_ << endl;
    cout << "Колонки: ";
    for (size_t i = 0; i < columns_.size(); i++) {
        cout << columns_[i] << " ";
    }
    cout << endl;

    if (!where_conditions_.empty()) {
        cout << "Условия WHERE: " << endl;
        for (size_t i = 0; i < where_conditions_.size(); i++) {
            const auto& cond = where_conditions_[i];
            cout << "  " << cond.column_name << " ";

            // Выводим оператор
            switch (static_cast<Operator>(cond.operator_id)) {
            case EQUAL: cout << "="; break;
            case NOT_EQUAL: cout << "!="; break;
            case LESS: cout << "<"; break;
            case LESS_OR_EQUAL: cout << "<="; break;
            case GREATER: cout << ">"; break;
            case GREATER_OR_EQUAL: cout << ">="; break;
            case LIKE: cout << "LIKE"; break;
            case IN_OP: cout << "IN"; break;
            case BETWEEN: cout << "BETWEEN"; break;
            default: cout << "?";
            }

            cout << " ";

            // Выводим значение
            if (cond.compare) {
                switch (cond.type) {
                case All_types::BIT:
                    cout << (*(static_cast<bool*>(cond.compare)) ? "true" : "false");
                    break;
                case All_types::TINYINT:
                    cout << static_cast<int>(*(static_cast<uint8_t*>(cond.compare)));
                    break;
                case All_types::SMALLINT:
                    cout << *(static_cast<int16_t*>(cond.compare));
                    break;
                case All_types::INT_TYPE:
                    cout << *(static_cast<int32_t*>(cond.compare));
                    break;
                case All_types::BIGINT:
                    cout << *(static_cast<int64_t*>(cond.compare));
                    break;
                case All_types::FLOAT_TYPE:
                    cout << *(static_cast<float*>(cond.compare));
                    break;
                case All_types::REAL:
                    cout << *(static_cast<double*>(cond.compare));
                    break;
                case All_types::CHAR_TYPE:
                case All_types::VARCHAR:
                case All_types::TEXT_TYPE:
                    cout << static_cast<char*>(cond.compare);
                    break;
                case All_types::DATETIME:
                case All_types::SMALLDATETIME:
                case All_types::DATE_TYPE:
                case All_types::TIME_TYPE:
                    // Для дат и времени выводим как есть (бинарные данные)
                    cout << "[данные даты/времени]";
                    break;
                default:
                    cout << "[бинарные данные]";
                }
            }
            else {
                cout << "NULL";
            }
            cout << endl;
        }
    }
}

string SelectCommand::getCommandType() const {
    return "SELECT";
}

bool SelectCommand::validate() {
    if (table_name_.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }
    return true;
}

// ================= UpdateCommand =================

UpdateCommand::UpdateCommand(const string& table_name,
    const vector<pair<string, string>>& set_clauses,
    const vector<Condition>& where_conditions)
    : table_name_(table_name), set_clauses_(set_clauses), where_conditions_(where_conditions) {
}

void UpdateCommand::execute(SimpleDB* db) {
    if (!validate()) {
        throw typeError(INVALID_SYNTAX, "Ошибка валидации UPDATE команды");
    }

    // Проверяем существование таблицы
    if (!db->tableExists(table_name_)) {
        throw typeError(TABLE_NOT_EXISTS, "Таблица '" + table_name_ + "' не существует");
    }

    cout << "Обновляем таблицу: " << table_name_ << endl;
    cout << "SET операции:" << endl;
    for (size_t i = 0; i < set_clauses_.size(); i++) {
        const auto& set_clause = set_clauses_[i];
        cout << "  " << set_clause.first << " = " << set_clause.second << endl;
    }

    if (!where_conditions_.empty()) {
        cout << "Условия WHERE: " << endl;
        for (size_t i = 0; i < where_conditions_.size(); i++) {
            const auto& cond = where_conditions_[i];
            cout << "  " << cond.column_name << " ";

            switch (static_cast<Operator>(cond.operator_id)) {
            case EQUAL: cout << "="; break;
            case NOT_EQUAL: cout << "!="; break;
            case LESS: cout << "<"; break;
            case LESS_OR_EQUAL: cout << "<="; break;
            case GREATER: cout << ">"; break;
            case GREATER_OR_EQUAL: cout << ">="; break;
            case LIKE: cout << "LIKE"; break;
            default: cout << "?";
            }

            cout << " ";

            if (cond.compare) {
                switch (cond.type) {
                case All_types::VARCHAR:
                case All_types::TEXT_TYPE:
                case All_types::CHAR_TYPE:
                    cout << static_cast<char*>(cond.compare);
                    break;
                case All_types::INT_TYPE:
                    cout << *(static_cast<int32_t*>(cond.compare));
                    break;
                default:
                    cout << "[значение]";
                }
            }
            else {
                cout << "NULL";
            }
            cout << endl;
        }
    }
}

string UpdateCommand::getCommandType() const {
    return "UPDATE";
}

bool UpdateCommand::validate() {
    if (table_name_.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }
    if (set_clauses_.empty()) {
        throw typeError(INVALID_SYNTAX, "UPDATE должен содержать хотя бы одну SET операцию");
    }

    for (size_t i = 0; i < set_clauses_.size(); i++) {
        if (set_clauses_[i].first.empty()) {
            throw typeError(INVALID_SYNTAX, "Имя колонки в SET операции не может быть пустым");
        }
    }

    return true;
}

// ================= DeleteCommand =================

DeleteCommand::DeleteCommand(const string& table_name,
    const vector<Condition>& where_conditions)
    : table_name_(table_name), where_conditions_(where_conditions) {
}

void DeleteCommand::execute(SimpleDB* db) {
    if (!validate()) {
        throw typeError(INVALID_SYNTAX, "Ошибка валидации DELETE команды");
    }

    if (!db->tableExists(table_name_)) {
        throw typeError(TABLE_NOT_EXISTS, "Таблица '" + table_name_ + "' не существует");
    }

    cout << "Удаляем из таблицы: " << table_name_ << endl;

    if (!where_conditions_.empty()) {
        cout << "Условия WHERE: " << endl;
        for (size_t i = 0; i < where_conditions_.size(); i++) {
            const auto& cond = where_conditions_[i];
            cout << "  " << cond.column_name << " ";

            switch (static_cast<Operator>(cond.operator_id)) {
            case Operator::EQUAL: cout << "="; break;
            case Operator::NOT_EQUAL: cout << "!="; break;
            case Operator::LESS: cout << "<"; break;
            case Operator::LESS_OR_EQUAL: cout << "<="; break;
            case Operator::GREATER: cout << ">"; break;
            case Operator::GREATER_OR_EQUAL: cout << ">="; break;
            case LIKE: cout << "LIKE"; break;
            default: cout << "?";
            }

            cout << " ";

            if (cond.compare) {
                switch (cond.type) {
                case All_types::VARCHAR:
                case All_types::TEXT_TYPE:
                case All_types::CHAR_TYPE:
                    cout << static_cast<char*>(cond.compare);
                    break;
                default:
                    cout << "[значение]";
                }
            }
            else {
                cout << "NULL";
            }
            cout << endl;
        }
    }
}

string DeleteCommand::getCommandType() const {
    return "DELETE";
}

bool DeleteCommand::validate() {
    if (table_name_.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }
    return true;
}

// ================= InsertCommand =================

InsertCommand::InsertCommand(const string& table_name,
    const vector<string>& column_names,
    const vector<vector<string>>& values)
    : table_name_(table_name), column_names_(column_names), values_(values) {
}

void InsertCommand::execute(SimpleDB* db) {
    if (!validate()) {
        throw typeError(INVALID_SYNTAX, "Ошибка валидации INSERT команды");
    }

    if (!db->tableExists(table_name_)) {
        throw typeError(TABLE_NOT_EXISTS, "Таблица '" + table_name_ + "' не существует");
    }

    cout << "Вставляем данные в таблицу: " << table_name_ << endl;

    if (!column_names_.empty()) {
        cout << "Колонки: ";
        for (size_t i = 0; i < column_names_.size(); i++) {
            cout << column_names_[i] << " ";
        }
        cout << endl;
    }

    for (size_t i = 0; i < values_.size(); i++) {
        cout << "Строка " << (i + 1) << ": (";
        for (size_t j = 0; j < values_[i].size(); j++) {
            cout << values_[i][j];
            if (j < values_[i].size() - 1) cout << ", ";
        }
        cout << ")" << endl;
    }
}

string InsertCommand::getCommandType() const {
    return "INSERT";
}

bool InsertCommand::validate() {
    if (table_name_.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }
    if (values_.empty()) {
        throw typeError(INVALID_SYNTAX, "INSERT должен содержать хотя бы одну строку значений");
    }

    size_t expected_values = column_names_.empty() ? 0 : column_names_.size();
    for (size_t i = 0; i < values_.size(); i++) {
        const auto& row = values_[i];
        if (!column_names_.empty() && row.size() != column_names_.size()) {
            throw typeError(INVALID_SYNTAX, "Количество значений не совпадает с количеством колонок");
        }
        if (column_names_.empty() && expected_values == 0) {
            expected_values = row.size();
        }
        else if (row.size() != expected_values) {
            throw typeError(INVALID_SYNTAX, "Все строки значений должны содержать одинаковое количество элементов");
        }
    }

    return true;
}

// ================= AlterCommand =================

AlterCommand::AlterCommand(const string& table_name,
    OperationType operation_type,
    const string& column_name,
    const string& data_type,
    const string& constraint,
    bool nullable)
    : table_name_(table_name), operation_type_(operation_type),
    column_name_(column_name), data_type_(data_type),
    constraint_(constraint), nullable_(nullable) {
}

void AlterCommand::execute(SimpleDB* db) {
    if (!validate()) {
        throw typeError(INVALID_SYNTAX, "Ошибка валидации ALTER команды");
    }

    if (!db->tableExists(table_name_)) {
        throw typeError(TABLE_NOT_EXISTS, "Таблица '" + table_name_ + "' не существует");
    }

    cout << "Изменяем таблицу: " << table_name_ << endl;
    cout << "Тип операции: ";
    if (operation_type_ == ADD_COLUMN) {
        cout << "ADD COLUMN" << endl;
        cout << "Новая колонка: " << column_name_ << " " << data_type_
            << (nullable_ ? " NULL" : " NOT NULL") << endl;
    }
    else if (operation_type_ == DROP_COLUMN) {
        cout << "DROP COLUMN" << endl;
        cout << "Удаляемая колонка: " << column_name_ << endl;
    }
    else {
        cout << "UNKNOWN" << endl;
    }
}

string AlterCommand::getCommandType() const {
    return "ALTER TABLE";
}

bool AlterCommand::validate() {
    if (table_name_.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }

    if (operation_type_ == ADD_COLUMN) {
        if (column_name_.empty() || data_type_.empty()) {
            throw typeError(INVALID_SYNTAX, "ADD_COLUMN требует указания имени колонки и типа данных");
        }
    }
    else if (operation_type_ == DROP_COLUMN) {
        if (column_name_.empty()) {
            throw typeError(INVALID_SYNTAX, "DROP_COLUMN требует указания имени колонки");
        }
    }

    return true;
}