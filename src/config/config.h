#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "types/types.h"  
#include "types/tests/validator.h"     // Валидатор
#include "types/tests/serializer.h"    // Сериализатор
#include "metadata/metadata.h"         // Метаданные
#include "parse/parser.h"
#include "command/Command.h"
#include "typeError/typeError.h" 
#include "condition/condition.h"

#define TABLE_DATA_PATH(table_name) (std::string("data/") + table_name + ".dat")
#define TABLE_META_PATH(table_name) (std::string("data/") + table_name + ".meta")

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

namespace Paths {
    constexpr const char* DATA_DIR = "data";
    constexpr const char* LOGS_DIR = "logs";
    constexpr const char* TMP_DIR = "tmp";
}

#endif