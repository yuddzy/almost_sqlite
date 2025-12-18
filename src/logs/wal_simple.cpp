#include "wal_simple.h"
#include <iostream>

namespace SimpleDB {

    std::string LogRecord::toString() const {
        std::ostringstream oss;
        oss << lsn << "|"
            << timestamp << "|"
            << txn_id << "|"
            << static_cast<int>(operation) << "|"
            << table << "|"
            << key << "|"
            << old_value << "|"
            << new_value;
        return oss.str();
    }

    uint64_t WriteAheadLog::getCurrentTimestamp() const {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto micros = duration_cast<microseconds>(now.time_since_epoch());
        return static_cast<uint64_t>(micros.count());
    }

    std::string WriteAheadLog::operationToString(Operation op) const {
        switch (op) {
        case Operation::INSERT: return "INSERT";
        case Operation::UPDATE: return "UPDATE";
        case Operation::DELETE: return "DELETE";
        case Operation::BEGIN: return "BEGIN";
        case Operation::COMMIT: return "COMMIT";
        case Operation::ROLLBACK: return "ROLLBACK";
        default: return "UNKNOWN";
        }
    }

    WriteAheadLog::~WriteAheadLog() {
        close();
    }

    bool WriteAheadLog::initialize(const std::string& log_file_path) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (log_file_.is_open()) {
            log_file_.close();
        }

        log_file_.open(log_file_path, std::ios::out | std::ios::app);
        if (!log_file_.is_open()) {
            std::cerr << "Failed to open log file: " << log_file_path << std::endl;
            return false;
        }

        // Записываем заголовок при создании файла
        if (log_file_.tellp() == 0) {
            log_file_ << "# WAL Log File\n";
            log_file_ << "# Format: LSN|Timestamp|TxnID|Operation|Table|Key|OldValue|NewValue\n";
            log_file_.flush();
        }

        std::cout << "WAL initialized: " << log_file_path << std::endl;
        return true;
    }

    uint64_t WriteAheadLog::log(Operation operation,
        const std::string& txn_id,
        const std::string& table,
        const std::string& key,
        const std::string& old_value,
        const std::string& new_value) {
        if (!enabled_) return 0;

        std::lock_guard<std::mutex> lock(mutex_);

        if (!log_file_.is_open()) {
            std::cerr << "WAL not initialized!" << std::endl;
            return 0;
        }

        uint64_t lsn = next_lsn_++;
        uint64_t timestamp = getCurrentTimestamp();

        // Экранируем специальные символы в значениях
        auto escape = [](const std::string& s) -> std::string {
            std::string result;
            for (char c : s) {
                if (c == '|' || c == '\n' || c == '\r') {
                    result += ' ';
                }
                else {
                    result += c;
                }
            }
            return result;
            };

        // Записываем в файл
        log_file_ << lsn << "|"
            << timestamp << "|"
            << txn_id << "|"
            << static_cast<int>(operation) << "|"
            << table << "|"
            << escape(key) << "|"
            << escape(old_value) << "|"
            << escape(new_value) << "\n";

        return lsn;
    }

    // Вспомогательные методы
    uint64_t WriteAheadLog::logBegin(const std::string& txn_id) {
        return log(Operation::BEGIN, txn_id, "", "", "", "");
    }

    uint64_t WriteAheadLog::logCommit(const std::string& txn_id) {
        return log(Operation::COMMIT, txn_id, "", "", "", "");
    }

    uint64_t WriteAheadLog::logRollback(const std::string& txn_id) {
        return log(Operation::ROLLBACK, txn_id, "", "", "", "");
    }

    uint64_t WriteAheadLog::logInsert(const std::string& txn_id,
        const std::string& table,
        const std::string& key,
        const std::string& value) {
        return log(Operation::INSERT, txn_id, table, key, "", value);
    }

    uint64_t WriteAheadLog::logUpdate(const std::string& txn_id,
        const std::string& table,
        const std::string& key,
        const std::string& old_value,
        const std::string& new_value) {
        return log(Operation::UPDATE, txn_id, table, key, old_value, new_value);
    }

    uint64_t WriteAheadLog::logDelete(const std::string& txn_id,
        const std::string& table,
        const std::string& key,
        const std::string& value) {
        return log(Operation::DELETE, txn_id, table, key, value, "");
    }

    void WriteAheadLog::flush() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.flush();
        }
    }

    void WriteAheadLog::close() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.flush();
            log_file_.close();
        }
    }

} // namespace SimpleDB