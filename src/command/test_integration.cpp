#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <memory>
#include <windows.h>
#include "Command.h"
#include "parser.h"
#include "db.h"

#include "../config/config.h" 
#include "../parse/parser.h"

using namespace std;

int main_main7() {
    SetConsoleOutputCP(65001);

    SimpleDB db;
    string command;

    cout << "=== Тестирование с новой структурой Condition ===" << endl;

    // Тестовые команды
    vector<string> test_commands = {
        "CREATE TABLE users (id INT_TYPE, name VARCHAR(50), age INT_TYPE)",
        "INSERT INTO users (id, name, age) VALUES (1, 'John', 25)",
        "SELECT * FROM users WHERE id = 1",
        "UPDATE users SET name = 'Jane' WHERE age > 20",
        "DELETE FROM users WHERE name LIKE 'J%'"
    };

    for (const auto& cmd : test_commands) {
        cout << "\nSQL> " << cmd << endl;

        try {
            auto sql_command = parse_sql_command(cmd);
            cout << "✓ Команда: " << sql_command->getCommandType() << endl;

            if (sql_command->validate()) {
                cout << "✓ Валидация прошла успешно" << endl;
                sql_command->execute(&db);
                cout << "✓ Команда выполнена" << endl;
            }
        }
        catch (const typeError& e) {
            std::cout << "✗ Ошибка: " << e.what() << std::endl;
        }
    }

    return 0;
}