#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>
#include "../types/types.h"
#include "../condition/condition.h"

// Предварительные объявления
class Command;
class CreateCommand;
class SelectCommand;
class InsertCommand;
class UpdateCommand;
class DeleteCommand;
class AlterCommand;

// Объявления функций - БЕЗ inline
Condition parse_single_where_condition(const std::string& condition);
std::vector<Condition> parse_where_conditions(const std::string& where_str);
int parse_command(const std::string& command);

std::unique_ptr<CreateCommand> parse_create_table_query(const std::string& command);
std::unique_ptr<SelectCommand> parse_select_query(const std::string& command);
std::unique_ptr<InsertCommand> parse_insert_query(const std::string& command);
std::unique_ptr<UpdateCommand> parse_update_query(const std::string& command);
std::unique_ptr<DeleteCommand> parse_delete_query(const std::string& command);
std::unique_ptr<AlterCommand> parse_alter_query(const std::string& command);

std::unique_ptr<Command> parse_sql_command(const std::string& command);

#endif // PARSER_H