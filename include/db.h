#ifndef INCLUDE_DB_H_
#define INCLUDE_DB_H_

#include "header.h"
#include "options.h"

namespace kv_benchmark {
class DB {
public:
    static void Open(Options& options, DB** dbptr);

    static void Close(DB* dbptr);

public:
    // Put key-value pair into database.
    // If "key" already exists, it will return false,
    // returns OK on success, and a non-OK status on error.
    virtual bool Put(char* key, size_t key_length, char* value, size_t value_length) = 0;

    // Update key-value pair which has existed in database,
    // If "key" not exists, it will return false.
    virtual bool Update(char* key, size_t key_length, char* value, size_t value_length) = 0;

public:
    // Get key-value pair from database.
    virtual bool Get(char* key, size_t key_length, char* value, size_t& value_length) = 0;

    // From key to get <scan_count> key-value pairs.
    virtual bool Scan(char* key, size_t key_length, size_t scan_count) = 0;

public:
    virtual void Print() = 0;
};
};

#endif