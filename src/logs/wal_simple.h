#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <atomic>

namespace SimpleDB {

    // Типы операций
    enum class Operation {
        INSERT,
        UPDATE,
        DELETE,
        BEGIN,
        COMMIT,
        ROLLBACK
    };

    // Структура для хранения записи лога
    struct LogRecord {
        uint64_t lsn;          // Sequence Number
        uint64_t timestamp;    // Время в микросекундах
        std::string txn_id;    // ID транзакции
        Operation operation;   // Тип операции
        std::string table;     // Имя таблицы
        std::string key;       // Ключ записи
        std::string old_value; // Старое значение (JSON)
        std::string new_value; // Новое значение (JSON)

        // Конвертация в строку (CSV-like формат для простоты)
        std::string toString() const;
    };

    class WriteAheadLog {
    private:
        std::ofstream log_file_;
        std::mutex mutex_;
        std::atomic<uint64_t> next_lsn_{ 1 };
        bool enabled_{ true };

        // Получить текущее время в микросекундах
        uint64_t getCurrentTimestamp() const;

        // Конвертировать операцию в строку
        std::string operationToString(Operation op) const;

    public:
        WriteAheadLog() = default;
        ~WriteAheadLog();

        // Инициализация
        bool initialize(const std::string& log_file_path);

        // Отключить/включить логирование
        void setEnabled(bool enabled) { enabled_ = enabled; }

        // Основной метод логирования
        uint64_t log(Operation operation,
            const std::string& txn_id,
            const std::string& table,
            const std::string& key = "",
            const std::string& old_value = "",
            const std::string& new_value = "");

        // Вспомогательные методы для частых операций
        uint64_t logBegin(const std::string& txn_id);
        uint64_t logCommit(const std::string& txn_id);
        uint64_t logRollback(const std::string& txn_id);
        uint64_t logInsert(const std::string& txn_id,
            const std::string& table,
            const std::string& key,
            const std::string& value);
        uint64_t logUpdate(const std::string& txn_id,
            const std::string& table,
            const std::string& key,
            const std::string& old_value,
            const std::string& new_value);
        uint64_t logDelete(const std::string& txn_id,
            const std::string& table,
            const std::string& key,
            const std::string& value);

        // Синхронизация с диском
        void flush();

        // Закрытие файла
        void close();
    };

} // namespace SimpleDB