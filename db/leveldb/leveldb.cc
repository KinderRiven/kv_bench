#include "leveldb.h"

using namespace kv_benchmark;

LevelDB::LevelDB(kv_benchmark::Options& options)
{
    leveldb::Options _options;
    _options.create_if_missing = true;
    leveldb::DB::Open(_options, options.db_path, &db_);
    assert(db_ != nullptr);
}

LevelDB::~LevelDB()
{
    delete db_;
}

bool LevelDB::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    leveldb::Slice _key(key, key_length);
    leveldb::Slice _value(value, value_length);
    leveldb::Status _status = db_->Put(leveldb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool LevelDB::Update(char* key, size_t key_length, char* value, size_t value_length)
{
    leveldb::Slice _key(key, key_length);
    leveldb::Slice _value(value, value_length);
    leveldb::Status _status = db_->Put(leveldb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool LevelDB::Get(char* key, size_t key_length, char* value, size_t& value_length)
{
    leveldb::Slice _key(key, key_length);
    std::string _value;
    leveldb::Status _status = db_->Get(leveldb::ReadOptions(), _key, &_value);
    return (_status.ok() == true) ? true : false;
}

bool LevelDB::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void LevelDB::Print()
{
}