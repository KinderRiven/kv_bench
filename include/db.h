/*
 * @Author: your name
 * @Date: 2021-03-18 11:55:19
 * @LastEditTime: 2021-03-18 19:08:04
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/include/db.h
 */
#ifndef INCLUDE_DB_H_
#define INCLUDE_DB_H_

#include "header.h"
#include "options.h"

namespace kv_benchmark {
class DB {
public:
    static void Open(Options& options, DB** dbptr);

    virtual void Close() = 0;

public:
    // Put key-value pair into database.
    // If "key" already exists, it will return false,
    // returns OK on success, and a non-OK status on error.
    virtual bool Put(char* key, size_t key_length, char* value, size_t value_length) = 0;

    // Update key-value pair which has existed in database,
    // If "key" not exists, it will return false.
    virtual bool Update(char* key, size_t key_length, char* value, size_t value_length) = 0;

    // Delete
    // Remove KV pair from DB
    virtual bool Delete(char* key, size_t key_length) = 0;

public:
    // Get key-value pair from database.
    virtual bool Get(char* key, size_t key_length, char* value, size_t& value_length) = 0;

    // Only for test
    // From key to get <scan_count> key-value pairs.
    virtual bool Scan(char* key, size_t key_length, size_t scan_count) = 0;

public:
    virtual void Print() = 0;
};
};

#endif