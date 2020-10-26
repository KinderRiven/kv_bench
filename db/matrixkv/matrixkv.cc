#include "matrixkv.h"

using namespace kv_benchmark;

MatrixKV::MatrixKV(kv_benchmark::Options& options)
{
    rocksdb::Options _options;
    _options.create_if_missing = true;
    _options.compression = rocksdb::kNoCompression;

    rocksdb::NvmSetup* _nvm_opt = new rocksdb::NvmSetup();
    _nvm_opt->pmem_path = "/home/pmem0";
    _options.nvm_setup.reset(_nvm_opt);
    rocksdb::DB::Open(_options, options.db_path, &db_);

    assert(db_ != nullptr);
}

MatrixKV::~MatrixKV()
{
    delete db_;
}

bool MatrixKV::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Update(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Get(char* key, size_t key_length, char* value, size_t& value_length)
{
    rocksdb::Slice _key(key, key_length);
    std::string _value;
    rocksdb::Status _status = db_->Get(rocksdb::ReadOptions(), _key, &_value);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void MatrixKV::Print()
{
}