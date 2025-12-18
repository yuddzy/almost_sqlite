#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>
#include <memory>
#include "parser.h"
#include "../config/config.h"
#include "../types/types.h"
#include "../condition/condition.h"
#include "../typeError/typeError.h"
#include "../command/Command.h"

using namespace std;

Condition parse_single_where_condition(const std::string& condition) {
    string cond = condition;
    cond.erase(0, cond.find_first_not_of(" "));
    cond.erase(cond.find_last_not_of(" ") + 1);

    // Определяем оператор
    size_t op_pos = string::npos;
    string op_str;
    Operator op = EQUAL;

    // Проверяем операторы в порядке приоритета
    vector<pair<string, Operator>> operators = {
        {"!=", NOT_EQUAL},
        {"<=", LESS_OR_EQUAL},
        {">=", GREATER_OR_EQUAL},
        {"=", EQUAL},
        {"<", LESS},
        {">", GREATER},
        {"LIKE", LIKE}
    };

    for (const auto& op_pair : operators) {
        size_t pos = cond.find(op_pair.first);
        if (pos != string::npos) {
            // Проверяем, что оператор не является частью слова
            if (pos > 0 && isalnum(cond[pos - 1])) continue;
            if (pos + op_pair.first.length() < cond.length() &&
                isalnum(cond[pos + op_pair.first.length()])) continue;

            op_pos = pos;
            op_str = op_pair.first;
            op = op_pair.second;
            break;
        }
    }

    if (op_pos == string::npos) {
        throw typeError(INVALID_SYNTAX, "Неизвестный оператор в условии WHERE: " + condition);
    }

    string column_name = cond.substr(0, op_pos);
    string value = cond.substr(op_pos + op_str.length());

    // Убираем пробелы
    column_name.erase(0, column_name.find_first_not_of(" "));
    column_name.erase(column_name.find_last_not_of(" ") + 1);
    value.erase(0, value.find_first_not_of(" "));
    value.erase(value.find_last_not_of(" ") + 1);

    // Если значение в кавычках, убираем их
    if (value.size() >= 2 && value[0] == '\'' && value.back() == '\'') {
        value = value.substr(1, value.length() - 2);
    }

    // Определяем тип значения и создаем указатель
    All_types value_type = All_types::VARCHAR;
    void* compare_value = nullptr;

    if (value.empty()) {
        value_type = All_types::VARCHAR;
        compare_value = malloc(1);
        static_cast<char*>(compare_value)[0] = '\0';
    }
    else if (value == "NULL") {
        value_type = All_types::INT_TYPE;
        compare_value = nullptr;
    }
    else {
        // Для простоты сохраняем как строку
        value_type = All_types::VARCHAR;
        compare_value = malloc(value.length() + 1);
        strcpy(static_cast<char*>(compare_value), value.c_str());
    }

    return create_condition(column_name, op, value_type, compare_value);
}

// Функция для парсинга нескольких условий WHERE
vector<Condition> parse_where_conditions(const string& where_str) {
    vector<Condition> conditions;

    string upper_where = where_str;
    transform(upper_where.begin(), upper_where.end(), upper_where.begin(), ::toupper);

    size_t start = 0;
    size_t and_pos;

    do {
        and_pos = upper_where.find(" AND ", start);

        string condition;
        if (and_pos != string::npos) {
            condition = where_str.substr(start, and_pos - start);
            start = and_pos + 5;
        }
        else {
            condition = where_str.substr(start);
        }

        // Убираем пробелы
        condition.erase(0, condition.find_first_not_of(" "));
        condition.erase(condition.find_last_not_of(" ") + 1);

        if (!condition.empty()) {
            conditions.push_back(parse_single_where_condition(condition));
        }

    } while (and_pos != string::npos);

    return conditions;
}

//функция возвращает тип команды
int parse_command(const string& command) {
    if (command.empty()) return -1;

    string upper_command = command;
    transform(upper_command.begin(), upper_command.end(), upper_command.begin(), ::toupper);

    size_t first_char = upper_command.find_first_not_of(" ");
    if (first_char == string::npos) return -1;

    if (upper_command.compare(first_char, 6, "CREATE") == 0) {
        return 0;
    }
    else if (upper_command.compare(first_char, 6, "SELECT") == 0) {
        return 1;
    }
    else if (upper_command.compare(first_char, 6, "UPDATE") == 0) {
        return 2;
    }
    else if (upper_command.compare(first_char, 6, "DELETE") == 0) {
        return 3;
    }
    else if (upper_command.compare(first_char, 6, "INSERT") == 0) {
        return 4;
    }
    else if (upper_command.compare(first_char, 5, "ALTER") == 0) {
        return 5;
    }

    return -1;
}

// Функция для создания CreateCommand из парсера
unique_ptr<CreateCommand> parse_create_table_query(const string& command) {
    string table_name;
    vector<string> column_names;
    vector<string> data_types;
    vector<bool> is_nullable;

    try {
        size_t table_start = command.find("TABLE") + 5;
        size_t paren_start = command.find("(");

        if (table_start == string::npos || paren_start == string::npos) {
            throw typeError(INVALID_SYNTAX, "Неверный синтаксис CREATE TABLE");
        }

        table_name = command.substr(table_start, paren_start - table_start);
        table_name.erase(0, table_name.find_first_not_of(" "));
        table_name.erase(table_name.find_last_not_of(" ") + 1);

        if (table_name.empty()) {
            throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
        }

        string columns_part = command.substr(paren_start + 1, command.find_last_of(")") - paren_start - 1);
        columns_part.erase(0, columns_part.find_first_not_of(" "));
        columns_part.erase(columns_part.find_last_not_of(" ") + 1);

        stringstream ss(columns_part);
        string column_definition;
        vector<string> parsed_column_names;

        while (getline(ss, column_definition, ',')) {
            column_definition.erase(0, column_definition.find_first_not_of(" "));
            column_definition.erase(column_definition.find_last_not_of(" ") + 1);

            if (column_definition.empty()) continue;

            size_t first_space = column_definition.find(" ");
            if (first_space == string::npos) {
                throw typeError(INVALID_SYNTAX, "Неверное определение колонки: " + column_definition);
            }

            string column_name = column_definition.substr(0, first_space);
            string column_type_str = column_definition.substr(first_space + 1);

            column_type_str.erase(0, column_type_str.find_first_not_of(" "));
            column_type_str.erase(column_type_str.find_last_not_of(" ") + 1);

            if (find(parsed_column_names.begin(), parsed_column_names.end(), column_name) != parsed_column_names.end()) {
                throw typeError(INVALID_SYNTAX, "Дублирующееся имя колонки: " + column_name);
            }
            parsed_column_names.push_back(column_name);

            string base_type = column_type_str;
            int size = 0;

            size_t paren_open = column_type_str.find("(");
            if (paren_open != string::npos) {
                base_type = column_type_str.substr(0, paren_open);
                size_t paren_close = column_type_str.find(")");
                if (paren_close != string::npos) {
                    string size_str = column_type_str.substr(paren_open + 1, paren_close - paren_open - 1);
                    try {
                        size = stoi(size_str);
                    }
                    catch (const exception&) {
                        throw typeError(INVALID_SYNTAX, "Неверный размер для колонки: " + column_name);
                    }
                }
            }

            All_types column_type_id = get_type_from_string(base_type);

            if (column_type_id == All_types::VARCHAR && size <= 0) {
                throw typeError(INVALID_SYNTAX, "VARCHAR должен иметь размер для колонки " + column_name);
            }

            if (column_type_id == All_types::CHAR_TYPE && size <= 0) {
                size = 1;
            }

            column_names.push_back(column_name);
            data_types.push_back(column_type_str);
            is_nullable.push_back(true);
        }

        if (column_names.empty()) {
            throw typeError(INVALID_SYNTAX, "Таблица должна иметь хотя бы одну колонку");
        }

    }
    catch (const typeError&) {
        throw;
    }
    catch (const exception& e) {
        throw typeError(INVALID_SYNTAX, "Ошибка парсинга CREATE TABLE: " + string(e.what()));
    }

    return make_unique<CreateCommand>(table_name, column_names, data_types, is_nullable);
}

// Функция для создания SelectCommand
unique_ptr<SelectCommand> parse_select_query(const string& command) {
    string upper_command = command;
    transform(upper_command.begin(), upper_command.end(), upper_command.begin(), ::toupper);

    size_t select_pos = upper_command.find("SELECT");
    if (select_pos == string::npos) {
        throw typeError(INVALID_SYNTAX, "Отсутствует SELECT в запросе");
    }

    size_t from_pos = upper_command.find("FROM");
    if (from_pos == string::npos) {
        throw typeError(INVALID_SYNTAX, "Отсутствует FROM в SELECT запросе");
    }

    // Извлекаем список колонок
    string columns_str = command.substr(select_pos + 6, from_pos - (select_pos + 6));
    columns_str.erase(0, columns_str.find_first_not_of(" \t"));
    columns_str.erase(columns_str.find_last_not_of(" \t") + 1);

    // Проверяем, что есть колонки
    if (columns_str.empty()) {
        throw typeError(INVALID_SYNTAX, "SELECT должен содержать список колонок или *");
    }

    vector<string> columns;
    stringstream ss_columns(columns_str);
    string column;
    while (getline(ss_columns, column, ',')) {
        column.erase(0, column.find_first_not_of(" \t"));
        column.erase(column.find_last_not_of(" \t") + 1);
        if (!column.empty()) {
            columns.push_back(column);
        }
    }

    if (columns.empty()) {
        columns.push_back("*");
    }

    string table_name = command.substr(from_pos + 4);
    table_name.erase(0, table_name.find_first_not_of(" "));

    // Убираем возможные условия WHERE, ORDER BY и тд
    size_t where_pos_global = upper_command.find("WHERE", from_pos);
    size_t order_pos = upper_command.find("ORDER BY", from_pos);
    size_t limit_pos = upper_command.find("LIMIT", from_pos);

    size_t end_pos = string::npos;
    if (where_pos_global != string::npos) end_pos = min(end_pos, where_pos_global);
    if (order_pos != string::npos) end_pos = min(end_pos, order_pos);
    if (limit_pos != string::npos) end_pos = min(end_pos, limit_pos);

    if (end_pos != string::npos) {
        table_name = command.substr(from_pos + 4, end_pos - (from_pos + 4));
    }

    table_name.erase(0, table_name.find_first_not_of(" "));
    table_name.erase(table_name.find_last_not_of(" \t;") + 1);

    if (table_name.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }

    vector<Condition> where_conditions;
    //vector<string> order_by;               
    //int limit = -1;

    // Парсим WHERE если есть
    if (where_pos_global != string::npos) {
        size_t where_start = where_pos_global + 5;
        string where_clause = command.substr(where_start);

        // Убираем ORDER BY и LIMIT из WHERE условия
        size_t where_end = where_clause.length();
        size_t order_in_where = where_clause.find("ORDER BY");
        size_t limit_in_where = where_clause.find("LIMIT");

        if (order_in_where != string::npos) where_end = min(where_end, order_in_where);
        if (limit_in_where != string::npos) where_end = min(where_end, limit_in_where);

        where_clause = where_clause.substr(0, where_end);
        where_clause.erase(0, where_clause.find_first_not_of(" \t"));
        where_clause.erase(where_clause.find_last_not_of(" \t") + 1);

        if (!where_clause.empty()) {
            where_conditions = parse_where_conditions(where_clause);
        }
    }

    return make_unique<SelectCommand>(columns, table_name, where_conditions);
}

// Функция для создания InsertCommand
unique_ptr<InsertCommand> parse_insert_query(const string& command) {
    string upper_command = command;
    transform(upper_command.begin(), upper_command.end(), upper_command.begin(), ::toupper);

    size_t into_pos = upper_command.find("INTO");
    if (into_pos == string::npos) {
        throw typeError(INVALID_SYNTAX, "Отсутствует INTO в INSERT запросе");
    }

    size_t values_pos = upper_command.find("VALUES");
    if (values_pos == string::npos) {
        throw typeError(INVALID_SYNTAX, "Отсутствует VALUES в INSERT запросе");
    }

    string table_part = command.substr(into_pos + 4, values_pos - (into_pos + 4));
    table_part.erase(0, table_part.find_first_not_of(" "));
    table_part.erase(table_part.find_last_not_of(" ") + 1);

    string table_name;
    vector<string> column_names;

    // Проверяем есть ли список колонок
    size_t paren_open = table_part.find("(");
    if (paren_open != string::npos) {
        table_name = table_part.substr(0, paren_open);
        table_name.erase(0, table_name.find_first_not_of(" "));
        table_name.erase(table_name.find_last_not_of(" ") + 1);

        size_t paren_close = table_part.find(")");
        if (paren_close == string::npos) {
            throw typeError(INVALID_SYNTAX, "Незакрытые скобки в списке колонок INSERT");
        }

        string columns_str = table_part.substr(paren_open + 1, paren_close - paren_open - 1);
        stringstream ss(columns_str);
        string column;
        while (getline(ss, column, ',')) {
            column.erase(0, column.find_first_not_of(" "));
            column.erase(column.find_last_not_of(" ") + 1);
            if (!column.empty()) {
                column_names.push_back(column);
            }
        }
    }
    else {
        table_name = table_part;
    }

    if (table_name.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }

    // Парсим значения
    string values_part = command.substr(values_pos + 6);
    values_part.erase(0, values_part.find_first_not_of(" "));

    size_t values_start = values_part.find("(");
    size_t values_end = values_part.find(")");
    if (values_start == string::npos || values_end == string::npos) {
        throw typeError(INVALID_SYNTAX, "Неверный формат значений в INSERT");
    }

    string values_str = values_part.substr(values_start + 1, values_end - values_start - 1);
    vector<string> values;
    stringstream ss_values(values_str);
    string value;

    while (getline(ss_values, value, ',')) {
        value.erase(0, value.find_first_not_of(" "));
        value.erase(value.find_last_not_of(" ") + 1);

        // Убираем кавычки если есть
        if (!value.empty() && value[0] == '\'' && value.back() == '\'') {
            value = value.substr(1, value.length() - 2);
        }

        values.push_back(value);
    }

    vector<vector<string>> values_vector = { values };

    return make_unique<InsertCommand>(table_name, column_names, values_vector);
}

// Функция для создания UpdateCommand
unique_ptr<UpdateCommand> parse_update_query(const string& command) {
    string upper_command = command;
    transform(upper_command.begin(), upper_command.end(), upper_command.begin(), ::toupper);

    size_t set_pos = upper_command.find("SET");
    if (set_pos == string::npos) {
        throw typeError(INVALID_SYNTAX, "Отсутствует SET в UPDATE запросе");
    }

    string table_name = command.substr(6, set_pos - 6);
    table_name.erase(0, table_name.find_first_not_of(" "));
    table_name.erase(table_name.find_last_not_of(" ") + 1);

    if (table_name.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }

    string set_part;
    size_t where_pos = upper_command.find("WHERE");

    if (where_pos != string::npos) {
        set_part = command.substr(set_pos + 3, where_pos - (set_pos + 3));
    }
    else {
        set_part = command.substr(set_pos + 3);
    }

    vector<pair<string, string>> set_clauses;
    vector<Condition> where_conditions;

    stringstream ss_set(set_part);
    string assignment;
    while (getline(ss_set, assignment, ',')) {
        size_t eq_pos = assignment.find('=');
        if (eq_pos != string::npos) {
            string column = assignment.substr(0, eq_pos);
            string value = assignment.substr(eq_pos + 1);

            column.erase(0, column.find_first_not_of(" "));
            column.erase(column.find_last_not_of(" ") + 1);
            value.erase(0, value.find_first_not_of(" "));
            value.erase(value.find_last_not_of(" ") + 1);

            set_clauses.push_back({ column, value });
        }
    }

    // Парсим WHERE если есть
    if (where_pos != string::npos) {
        string where_part = command.substr(where_pos + 5);
        where_part.erase(0, where_part.find_first_not_of(" "));
        where_part.erase(where_part.find_last_not_of(" \t;") + 1);

        if (!where_part.empty()) {
            where_conditions = parse_where_conditions(where_part);
        }
    }

    return make_unique<UpdateCommand>(table_name, set_clauses, where_conditions);
}

// Функция для создания DeleteCommand
unique_ptr<DeleteCommand> parse_delete_query(const string& command) {
    string upper_command = command;
    transform(upper_command.begin(), upper_command.end(), upper_command.begin(), ::toupper);

    size_t from_pos = upper_command.find("FROM");
    if (from_pos == string::npos) {
        throw typeError(INVALID_SYNTAX, "Отсутствует FROM в DELETE запросе");
    }

    // Извлекаем часть после FROM
    string after_from = command.substr(from_pos + 4);
    after_from.erase(0, after_from.find_first_not_of(" "));

    string table_name;
    vector<Condition> where_conditions;

    size_t where_pos = upper_command.find("WHERE", from_pos + 4);

    if (where_pos != string::npos) {
        // Извлекаем имя таблицы 
        table_name = command.substr(from_pos + 4, where_pos - (from_pos + 4));
        table_name.erase(0, table_name.find_first_not_of(" "));
        table_name.erase(table_name.find_last_not_of(" \t") + 1);

        // Извлекаем условие WHERE
        string where_part = command.substr(where_pos + 5);
        where_part.erase(0, where_part.find_first_not_of(" "));
        where_part.erase(where_part.find_last_not_of(" \t;") + 1);

        if (!where_part.empty()) {
            //используем новую функцию парсинга
            where_conditions = parse_where_conditions(where_part);
        }
    }
    else {
        // Нет WHERE
        table_name = after_from;
        table_name.erase(table_name.find_last_not_of(" \t;") + 1);
    }
    // Очищаем имя таблицы от пробелов
    table_name.erase(0, table_name.find_first_not_of(" "));
    table_name.erase(table_name.find_last_not_of(" \t") + 1);

    if (table_name.empty()) {
        throw typeError(INVALID_SYNTAX, "Имя таблицы не может быть пустым");
    }

    return make_unique<DeleteCommand>(table_name, where_conditions);
}

// Функция для создания AlterCommand
unique_ptr<AlterCommand> parse_alter_query(const string& command) {
    string upper_command = command;
    transform(upper_command.begin(), upper_command.end(), upper_command.begin(), ::toupper);

    size_t pos = 0;
    while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;

    if (upper_command.substr(pos, 5) != "ALTER") {
        throw typeError(INVALID_SYNTAX, "Ожидается 'ALTER' в запросе: " + command);
    }
    pos += 5;

    while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;

    if (upper_command.substr(pos, 5) != "TABLE") {
        throw typeError(INVALID_SYNTAX, "Ожидается 'TABLE' в ALTER запросе: " + command);
    }
    pos += 5;

    while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;

    // Извлекаем имя таблицы
    size_t table_start = pos;
    while (pos < upper_command.length() && !isspace(upper_command[pos]) && upper_command[pos] != ';') {
        ++pos;
    }

    if (table_start == pos) {
        throw typeError(INVALID_SYNTAX, "Отсутствует имя таблицы в ALTER запросе: " + command);
    }

    string table_name = command.substr(table_start, pos - table_start);

    while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;

    // Определяем тип операции
    AlterCommand::OperationType operation_type;
    string column_name, data_type;

    if (upper_command.substr(pos, 3) == "ADD") {
        operation_type = AlterCommand::ADD_COLUMN;
        pos += 3;
        while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;

        // Пропускаем ключевое слово COLUMN если есть
        if (upper_command.substr(pos, 6) == "COLUMN") {
            pos += 6;
            while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;
        }

        // Извлекаем имя колонки
        size_t column_start = pos;
        while (pos < upper_command.length() && !isspace(upper_command[pos]) && upper_command[pos] != ';') {
            ++pos;
        }
        column_name = command.substr(column_start, pos - column_start);

        // Оставшаяся часть - тип данных
        while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;
        data_type = command.substr(pos);
    }
    else if (upper_command.substr(pos, 4) == "DROP") {
        operation_type = AlterCommand::DROP_COLUMN;
        pos += 4;
        while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;

        // Пропускаем ключевое слово COLUMN если есть
        if (upper_command.substr(pos, 6) == "COLUMN") {
            pos += 6;
            while (pos < upper_command.length() && isspace(upper_command[pos])) ++pos;
        }

        // Извлекаем имя колонки
        size_t column_start = pos;
        while (pos < upper_command.length() && !isspace(upper_command[pos]) && upper_command[pos] != ';') {
            ++pos;
        }
        column_name = command.substr(column_start, pos - column_start);
    }
    else {
        throw typeError(INVALID_SYNTAX, "Неизвестная операция в ALTER запросе: " + command);
    }

    return make_unique<AlterCommand>(table_name, operation_type, column_name, data_type, "", true);
}

// Главная функция парсера которая возвращает Command
unique_ptr<Command> parse_sql_command(const string& command) {
    int command_type = parse_command(command);

    switch (command_type) {
    case 0:
        return parse_create_table_query(command);
    case 1:
        return parse_select_query(command);
    case 2:
        return parse_update_query(command);
    case 3:
        return parse_delete_query(command);
    case 4:
        return parse_insert_query(command);
    case 5:
        return parse_alter_query(command);
    default:
        throw typeError(INVALID_SYNTAX, "Неизвестная или неподдерживаемая команда: " + command);
    }
}

int main() {
    string command;

    while (true) {
        cout << "\n=== Almost SQLite ===" << endl;
        cout << "Введите SQL команду (или 'EXIT' для выхода): ";
        getline(cin, command);

        if (command == "EXIT" || command == "exit") {
            cout << "Выход..." << endl;
            break;
        }

        if (command.empty()) continue;

        try {
            auto sql_command = parse_sql_command(command);
            cout << "Успешно распарсена команда: " << sql_command->getCommandType() << endl;

            // Здесь можно вызвать 

        }
        catch (const typeError& e) {
            cout << "Ошибка валидации: " << e.what() << endl;
        }
        catch (const exception& e) {
            cout << "Ошибка: " << e.what() << endl;
        }
    }

    return 0;
}