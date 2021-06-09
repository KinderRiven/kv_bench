/*
 * @Date: 2021-03-18 11:55:19
 * @LastEditors: Han Shukai
 * @LastEditTime: 2021-06-09 18:55:07
 * @FilePath: /kv_bench/db/leveldb/leveldb.h
 */
#ifndef INCLUDE_DB_LEVELDB_H_
#define INCLUDE_DB_LEVELDB_H_

// v1.20
#include "db.h"
#include "header.h"
#include "leveldb/db.h"
#include "options.h"

namespace kv_benchmark {
class LevelDB : public kv_benchmark::DB {
public:
    LevelDB(kv_benchmark::Options& options);

    ~LevelDB();

public:
    // Put key-value pair into database.
    // If "key" already exists, it will return false,
    // returns OK on success, and a non-OK status on error.
    bool Put(char* key, size_t key_length, char* value, size_t value_length);

    // Update key-value pair which has existed in database,
    // If "key" not exists, it will return false.
    bool Update(char* key, size_t key_length, char* value, size_t value_length);

public:
    // Get key-value pair from database.
    bool Get(char* key, size_t key_length, char* value, size_t& value_length);

    // From key to get <scan_count> key-value pairs.
    bool Scan(char* key, size_t key_length, size_t scan_count);

public:
    void Print();

private:
    leveldb::DB* db_;
};
};

#endif