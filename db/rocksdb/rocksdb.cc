#include "rocksdb.h"

using namespace kv_benchmark;

RocksDB::RocksDB(kv_benchmark::Options& options)
{
    rocksdb::Options _options;
    _options.create_if_missing = true;
    _options.compression = rocksdb::kNoCompression;
    rocksdb::DB::Open(_options, options.db_path, &db_);
    assert(db_ != nullptr);
}

RocksDB::~RocksDB()
{
    delete db_;
}

bool RocksDB::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool RocksDB::Update(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool RocksDB::Get(char* key, size_t key_length, char* value, size_t& value_length)
{
    rocksdb::Slice _key(key, key_length);
    std::string _value;
    rocksdb::Status _status = db_->Get(rocksdb::ReadOptions(), _key, &_value);
    return (_status.ok() == true) ? true : false;
}

bool RocksDB::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void RocksDB::Print()
{
}