#ifndef COMMAND_H
#define COMMAND_H

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "config/config.h" 
#include "typeError/typeError.h"
#include "condition/condition.h"

// Предварительное объявление SimpleDB
class SimpleDB;

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(SimpleDB* db) = 0;
    virtual std::string getCommandType() const = 0;
    virtual bool validate() = 0;
};

class CreateCommand : public Command {
private:
    std::string table_name_;
    std::vector<std::string> column_names_;
    std::vector<std::string> data_types_;
    std::vector<bool> is_nullable_;

public:
    CreateCommand(const std::string& table_name,
        const std::vector<std::string>& column_names,
        const std::vector<std::string>& data_types,
        const std::vector<bool>& is_nullable);

    void execute(SimpleDB* db) override;
    std::string getCommandType() const override;
    bool validate() override;
};

class SelectCommand : public Command {
private:
    std::vector<std::string> columns_;
    std::string table_name_;
    std::vector<Condition> where_conditions_;

public:
    SelectCommand(const std::vector<std::string>& columns,
        const std::string& table_name,
        const std::vector<Condition>& where_conditions);

    void execute(SimpleDB* db) override;
    std::string getCommandType() const override;
    bool validate() override;
};

class InsertCommand : public Command {
private:
    std::string table_name_;
    std::vector<std::string> column_names_;
    std::vector<std::vector<std::string>> values_;

public:
    InsertCommand(const std::string& table_name,
        const std::vector<std::string>& column_names,
        const std::vector<std::vector<std::string>>& values);

    void execute(SimpleDB* db) override;
    std::string getCommandType() const override;
    bool validate() override;
};

class UpdateCommand : public Command {
private:
    std::string table_name_;
    std::vector<std::pair<std::string, std::string>> set_clauses_;
    std::vector<Condition> where_conditions_;

public:
    UpdateCommand(const std::string& table_name,
        const std::vector<std::pair<std::string, std::string>>& set_clauses,
        const std::vector<Condition>& where_conditions);

    void execute(SimpleDB* db) override;
    std::string getCommandType() const override;
    bool validate() override;
};

class DeleteCommand : public Command {
private:
    std::string table_name_;
    std::vector<Condition> where_conditions_;

public:
    DeleteCommand(const std::string& table_name,
        const std::vector<Condition>& where_conditions);

    void execute(SimpleDB* db) override;
    std::string getCommandType() const override;
    bool validate() override;
};

class AlterCommand : public Command {
public:
    enum OperationType {
        ADD_COLUMN,
        DROP_COLUMN
    };

private:
    std::string table_name_;
    OperationType operation_type_;
    std::string column_name_;
    std::string data_type_;
    std::string constraint_;
    bool nullable_;

public:
    AlterCommand(const std::string& table_name,
        OperationType operation_type,
        const std::string& column_name,
        const std::string& data_type,
        const std::string& constraint,
        bool nullable);

    void execute(SimpleDB* db) override;
    std::string getCommandType() const override;
    bool validate() override;
};

#endif // COMMAND_H