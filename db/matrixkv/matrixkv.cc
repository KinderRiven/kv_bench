#include "matrixkv.h"
#include "rocksdb/table.h"

using namespace kv_benchmark;

MatrixKV::MatrixKV(kv_benchmark::Options& options)
{
    rocksdb::Options _options;
    _options.create_if_missing = true;
    _options.compression = rocksdb::kNoCompression;

    rocksdb::NvmSetup* _nvm_opt = new rocksdb::NvmSetup();
    _nvm_opt->pmem_path = "/home/pmem0";
    _nvm_opt->use_nvm_module = true;
    _options.nvm_setup.reset(_nvm_opt);

    // set block cache
    std::shared_ptr<rocksdb::Cache> _cache = rocksdb::NewLRUCache(8 * 1024 * 1024);
    rocksdb::BlockBasedTableOptions _table_options;
    _table_options.block_cache = _cache;
    _options.table_factory.reset(NewBlockBasedTableFactory(_table_options));

    // direct read/write
    _options.use_direct_reads = true;
    _options.use_direct_io_for_flush_and_compaction = true;

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
    value_length = _value.size();
    memcpy(value, _value.data(), value_length);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void MatrixKV::Print()
{
}