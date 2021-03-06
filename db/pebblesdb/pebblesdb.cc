/*
 * @Date: 2021-03-18 11:55:19
 * @LastEditors: Han Shukai
 * @LastEditTime: 2021-06-09 15:07:58
 * @FilePath: /kv_bench/db/pebblesdb/pebblesdb.cc
 */
#include "pebblesdb.h"

using namespace kv_benchmark;

PebblesDB::PebblesDB(kv_benchmark::Options& options)
{
    leveldb::Options _options;
    _options.create_if_missing = true;
    _options.compression = leveldb::kNoCompression;
    _options.write_buffer_size = 64 * 1024 * 1024;
    leveldb::DB::Open(_options, options.db_path, &db_);
    assert(db_ != nullptr);
}

PebblesDB::~PebblesDB()
{
    delete db_;
}

bool PebblesDB::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    leveldb::Slice _key(key, key_length);
    leveldb::Slice _value(value, value_length);
    leveldb::Status _status = db_->Put(leveldb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool PebblesDB::Update(char* key, size_t key_length, char* value, size_t value_length)
{
    leveldb::Slice _key(key, key_length);
    leveldb::Slice _value(value, value_length);
    leveldb::Status _status = db_->Put(leveldb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool PebblesDB::Get(char* key, size_t key_length, char* value, size_t& value_length)
{
    leveldb::Slice _key(key, key_length);
    std::string _value;
    leveldb::Status _status = db_->Get(leveldb::ReadOptions(), _key, &_value);
    value_length = _value.size();
    memcpy(value, _value.data(), value_length);
    return (_status.ok() == true) ? true : false;
}

bool PebblesDB::Delete(char* key, size_t key_length)
{
    return false;
}

bool PebblesDB::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void PebblesDB::Print()
{
}