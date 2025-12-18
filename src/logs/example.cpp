#include "wal_simple.h"

class SimpleDatabase {
private:
    SimpleDB::WriteAheadLog wal_;

public:
    SimpleDatabase(const std::string& log_path) {
        if (!wal_.initialize(log_path)) {
            throw std::runtime_error("Failed to initialize WAL");
        }
    }

    bool updateRecord(const std::string& txn_id,
        const std::string& table,
        const std::string& key,
        const std::string& new_value) {

        wal_.logBegin(txn_id);

        try {
            std::string old_value = getCurrentValue(table, key);

            wal_.logUpdate(txn_id, table, key, old_value, new_value);

            bool success = performUpdate(table, key, new_value);

            if (success) {
                wal_.logCommit(txn_id);
                wal_.flush(); 
                return true;
            }
            else {
                wal_.logRollback(txn_id);
                return false;
            }

        }
        catch (const std::exception& e) {
            wal_.logRollback(txn_id);
            wal_.flush();
            throw;
        }
    }

    bool insertRecord(const std::string& txn_id,
        const std::string& table,
        const std::string& key,
        const std::string& value) {

        wal_.logBegin(txn_id);

        try {
            wal_.logInsert(txn_id, table, key, value);

            bool success = performInsert(table, key, value);

            if (success) {
                wal_.logCommit(txn_id);
                wal_.flush();
                return true;
            }
            else {
                wal_.logRollback(txn_id);
                return false;
            }

        }
        catch (...) {
            wal_.logRollback(txn_id);
            wal_.flush();
            throw;
        }
    }

private:
    std::string getCurrentValue(const std::string& table, const std::string& key) {
		/// логика
        return "";
    }

    bool performUpdate(const std::string& table,
        const std::string& key,
        const std::string& value) {
        // логика обновления
        return true;
    }

    bool performInsert(const std::string& table,
        const std::string& key,
        const std::string& value) {
        //  логика вставки
        return true;
    }
};