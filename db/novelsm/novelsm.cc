#include "novelsm.h"

using namespace kv_benchmark;

NoveLSM::NoveLSM(kv_benchmark::Options& options)
{
    leveldb::Options _options;
    _options.create_if_missing = true;
    _options.block_size = 4096;
    _options.max_file_size = (64 * 1024 * 1024);
    _options.write_buffer_size = (64 * 1024 * 1024);
    _options.nvm_buffer_size = (256 * 1024 * 1024);
    leveldb::DB::Open(_options, options.db_path, options.nvm_path, &db_);
    assert(db_ != nullptr);
}

NoveLSM::~NoveLSM()
{
    delete db_;
}

bool NoveLSM::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    leveldb::Slice _key(key, key_length);
    leveldb::Slice _value(value, value_length);
    leveldb::Status _status = db_->Put(leveldb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool NoveLSM::Update(char* key, size_t key_length, char* value, size_t value_length)
{
    leveldb::Slice _key(key, key_length);
    leveldb::Slice _value(value, value_length);
    leveldb::Status _status = db_->Put(leveldb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool NoveLSM::Get(char* key, size_t key_length, char* value, size_t& value_length)
{
    leveldb::Slice _key(key, key_length);
    std::string _value;
    leveldb::Status _status = db_->Get(leveldb::ReadOptions(), _key, &_value);
    return (_status.ok() == true) ? true : false;
}

bool NoveLSM::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void NoveLSM::Print()
{
}