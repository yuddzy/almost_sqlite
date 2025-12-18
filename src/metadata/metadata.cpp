#define _CRT_SECURE_NO_WARNINGS
#include "metadata.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

void serialize_metadata(const string& table_name, const vector<Column>& columns, uint64_t record_count) {
    ofstream meta_file(table_name + ".meta", ios::binary);

    if (!meta_file.is_open()) {
        cerr << "Error: Cannot create metadata file for table " << table_name << endl;
        return;
    }

    TableMetadataHeader header;
    string signature_str = "ALMOST_SQLITE_V1";
    strncpy(header.signature, signature_str.c_str(), sizeof(header.signature) - 1);
    header.signature[sizeof(header.signature) - 1] = '\0';

    header.version = 1;
    header.column_count = static_cast<uint32_t>(columns.size());
    header.created_time = static_cast<uint64_t>(time(nullptr));
    header.record_count = record_count;
    header.data_file_size = 0;
    header.flags = 0;

    meta_file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    uint16_t current_offset = 0;
    for (const auto& column : columns) {
        ColumnMetadata col_meta;

        strncpy(col_meta.name, column.name.c_str(), sizeof(col_meta.name) - 1);
        col_meta.name[sizeof(col_meta.name) - 1] = '\0';

        col_meta.type = column.type;
        col_meta.size = static_cast<uint32_t>(column.size);
        col_meta.offset = current_offset;
        col_meta.flags = 0;

        current_offset += static_cast<uint16_t>(get_type_size(column.type, column.size));

        meta_file.write(reinterpret_cast<const char*>(&col_meta), sizeof(col_meta));
    }

    meta_file.close();
    cout << "Metadata file created: " << table_name << ".meta" << endl;
}

bool deserialize_metadata(const string& table_name, vector<Column>& columns, uint64_t& record_count) {
    ifstream meta_file(table_name + ".meta", ios::binary);

    if (!meta_file.is_open()) {
        return false;
    }

    TableMetadataHeader header;
    meta_file.read(reinterpret_cast<char*>(&header), sizeof(header));

    string read_signature(header.signature, sizeof(header.signature));
    string expected_signature = "ALMOST_SQLITE_V1";

    size_t null_pos = read_signature.find('\0');
    if (null_pos != string::npos) {
        read_signature = read_signature.substr(0, null_pos);
    }

    if (read_signature != expected_signature) {
        cerr << "Error: Invalid metadata signature. Got: '" << read_signature
            << "' Expected: '" << expected_signature << "'" << endl;
        return false;
    }

    columns.clear();
    record_count = header.record_count;

    for (uint32_t i = 0; i < header.column_count; i++) {
        ColumnMetadata col_meta;
        meta_file.read(reinterpret_cast<char*>(&col_meta), sizeof(col_meta));

        Column column(string(col_meta.name), col_meta.type, static_cast<int>(col_meta.size));

        columns.push_back(column);
    }

    meta_file.close();
    return true;
}