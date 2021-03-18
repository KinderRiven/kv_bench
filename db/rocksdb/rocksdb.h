/*
 * @Author: your name
 * @Date: 2021-03-18 11:55:19
 * @LastEditTime: 2021-03-18 12:10:06
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/db/rocksdb/rocksdb.h
 */
#ifndef INCLUDE_DB_ROCKSDB_H_
#define INCLUDE_DB_ROCKSDB_H_

#include "db.h"
#include "header.h"
#include "options.h"
#include "rocksdb/db.h"

namespace kv_benchmark {
class RocksDB : public kv_benchmark::DB {
public:
    RocksDB(kv_benchmark::Options& options);

    ~RocksDB();

public:
    // Put key-value pair into database.
    // If "key" already exists, it will return false,
    // returns OK on success, and a non-OK status on error.
    bool Put(char* key, size_t key_length, char* value, size_t value_length);

    // Update key-value pair which has existed in database,
    // If "key" not exists, it will return false.
    bool Update(char* key, size_t key_length, char* value, size_t value_length);

    bool Delete(char* key, size_t key_length);

public:
    // Get key-value pair from database.
    bool Get(char* key, size_t key_length, char* value, size_t& value_length);

    // From key to get <scan_count> key-value pairs.
    bool Scan(char* key, size_t key_length, size_t scan_count);

public:
    void Print();

private:
    // rocksdb::WriteOptions wopt_;
    // rocksdb::ReadOptions ropt_;
    rocksdb::DB* db_;
};
};

#endif