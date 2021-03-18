#include "rocksdb_benchtools.h"
#define OUTPUT_DEBUG 0

//#define STATUS_REPORT
#define MULTITHREAD_TEST

int64_t ConvertKeyToInt(std::string s, std::size_t key_size)
{
    int64_t ret = 0;
    for (std::size_t t = 0; t < key_size; ++t) {
        ret = (ret << (1 << 3)) + (int64_t)(unsigned char)(s[t]);
    }
    return ret;
}

namespace ROCKSDB_BENCHMARK_NAMESPACE {

SingleQuery::SingleQuery()
    : valid(false)
    , type(Get)
    , key(nullptr, 0)
    , val(nullptr, 0)
    , key_size(0)
    , val_size(0)
    , scan_len(0)
{
}

SingleQuery::SingleQuery(const SingleQuery& rhs)
    : valid(rhs.valid)
    , type(rhs.type)
    , key(rhs.key)
    , val(rhs.val)
    , key_size(rhs.key_size)
    , val_size(rhs.val_size)
    , scan_len(rhs.scan_len)
{
}

SingleQuery::~SingleQuery() {}

static unsigned int value_size = 100;

Benchmark::Benchmark(Options options)
    : bench_name(options.bench_name)
    , flags_num(options.flags_num)
    , read_random_exp_range_(options.read_random_exp_range)
    , keys_per_prefix_(options.keys_per_prefix)
    , key_size_(options.key_size)
    , rand(options.rand_seed)
    , flags_ops_between_duration_checks(options.flags_ops_between_duration_checks)
    , entries_per_batch(options.entries_per_batch)
    , flags_multiread_stride(options.flags_multiread_stride)
    , flags_value_size_min(options.flags_value_size_min)
    , flags_value_size_max(options.flags_value_size_max)
    , flags_mix_max_scan_len(options.flags_mix_max_scan_len)
    , flags_mix_max_value_size(options.flags_mix_max_value_size)
    , flags_avg_value_size(options.flags_avg_value_size)
    , flags_mix_get_ratio(options.flags_mix_get_ratio)
    , flags_mix_put_ratio(options.flags_mix_put_ratio)
    , flags_mix_seek_ratio(options.flags_mix_seek_ratio)
    , flags_keyrange_dist_a(options.flags_keyrange_dist_a)
    , flags_keyrange_dist_b(options.flags_keyrange_dist_b)
    , flags_keyrange_dist_c(options.flags_keyrange_dist_c)
    , flags_keyrange_dist_d(options.flags_keyrange_dist_d)
    , flags_key_dist_a(options.flags_key_dist_a)
    , flags_key_dist_b(options.flags_key_dist_b)
    , flags_value_theta(options.flags_value_theta)
    , flags_value_k(options.flags_value_k)
    , flags_value_sigma(options.flags_value_sigma)
    , flags_iter_theta(options.flags_iter_theta)
    , flags_iter_k(options.flags_iter_k)
    , flags_iter_sigma(options.flags_iter_sigma)
    , keyrange_num(options.keyrange_num)
    , monitor_range_num(options.monitor_range_num)
    , use_static_value_size(options.use_static_value_size)
    , gen(/* flags_value_size_min=*/options.flags_value_size_min,
          /* flags_value_size_max=*/options.flags_value_size_max,
          /* value_size=*/value_size,
          /* flags_compression_ratio=*/options.flags_compression_ratio)
    , counter(options.flags_num, options.monitor_range_num)
{
    if (value_max > flags_mix_max_value_size) {
        value_max = flags_mix_max_value_size;
    }
}

Status Benchmark::InitiateBenchmark()
{
    if (flags_keyrange_dist_a != 0.0 || flags_keyrange_dist_b != 0.0 || flags_keyrange_dist_c != 0.0 || flags_keyrange_dist_d != 0.0) {
        use_prefix_modeling = true;
        gen_exp.InitiateExpDistribution(/*flags_num=*/flags_num, /*keyrange_num=*/keyrange_num,
            /*total_keys=*/flags_num, /*prefix_a=*/flags_keyrange_dist_a,
            /*prefix_b=*/flags_keyrange_dist_b, /*prefix_c=*/flags_keyrange_dist_c,
            /*prefix_d=*/flags_keyrange_dist_d);
    }
    if (flags_key_dist_a == 0 || flags_key_dist_b == 0) {
        use_random_modeling = true;
    }
    if (bench_name == "MixGraph") {
        std::vector<double> ratio{ flags_mix_get_ratio, flags_mix_put_ratio, flags_mix_seek_ratio };
        query.Initiate(ratio);
    } else if (bench_name == "MixRandom") {
        std::vector<double> ratio{ flags_mix_get_ratio, flags_mix_put_ratio };
        query.Initiate(ratio);
    }
    counter.InitiateCounter();
    initiated = true;
    return Status::OK();
}

SingleQuery Benchmark::GetSingleQuery()
{
    SingleQuery ret;
    if (bench_name == "MixGraph") {
        ret = MixGraph();
    } else if (bench_name == "ReadRandom") {
        ret = ReadRandom();
    } else if (bench_name == "MixRandom") {
        ret = MixRandom();
    } else {
        ret = SingleQuery();
    }

    if (ret.type == Get) {
        counter.AddOpRecord(ret.type, ret.key.size(), ret.key);
    } else if (ret.type == Put) {
        counter.AddOpRecord(ret.type, ret.key.size() + ret.val.size(), ret.key);
    } else if (ret.type == SeekForPrev) {
        counter.AddOpRecord(ret.type, ret.key.size() * ret.scan_len, ret.key);
    }
    return ret;
}

void Benchmark::GenerateKeyFromInt(uint64_t v, int64_t num_keys, Slice* key)
{
    if (!keys_.empty()) {
        //assert(FLAGS_use_existing_keys);
        assert(keys_.size() == static_cast<size_t>(num_keys));
        assert(v < static_cast<uint64_t>(num_keys));
        *key = keys_[v];
        return;
    }
    char* start = const_cast<char*>(key->data());
    char* pos = start;
    if (keys_per_prefix_ > 0) {
        int64_t num_prefix = num_keys / keys_per_prefix_;
        int64_t prefix = v % num_prefix;
        int bytes_to_fill = std::min(prefix_size_, 8);
        if (kLittleEndian) {
            for (int i = 0; i < bytes_to_fill; ++i) {
                pos[i] = (prefix >> ((bytes_to_fill - i - 1) << 3)) & 0xFF;
            }
        } else {
            memcpy(pos, static_cast<void*>(&prefix), bytes_to_fill);
        }
        if (prefix_size_ > 8) {
            // fill the rest with 0s
            memset(pos + 8, '0', prefix_size_ - 8);
        }
        pos += prefix_size_;
    }

    int bytes_to_fill = std::min(key_size_ - static_cast<int>(pos - start), 8);
    if (kLittleEndian) {
        for (int i = 0; i < bytes_to_fill; ++i) {
            pos[i] = (v >> ((bytes_to_fill - i - 1) << 3)) & 0xFF;
        }
    } else {
        memcpy(pos, static_cast<void*>(&v), bytes_to_fill);
    }
    pos += bytes_to_fill;
    if (key_size_ > pos - start) {
        memset(pos, '0', key_size_ - (pos - start));
    }
}

Slice Benchmark::AllocateKey(std::unique_ptr<const char[]>* key_guard)
{
    char* data = new char[key_size_];
    const char* const_data = data;
    key_guard->reset(const_data);
    return Slice(key_guard->get(), key_size_);
}

void Benchmark::StatusReport()
{
    std::cout << "Bench Name: " << bench_name << std::endl;
    fprintf(stdout, "Total Key Numbers: %" PRId64 "\n", flags_num);
    fprintf(stdout, "Range Number: %" PRId64 "\n", keyrange_num);
    fprintf(stdout, "Key Size: %d bytes each\n", key_size_);
    counter.StatusReport();
}

int64_t Benchmark::GetRandomKey(Random64* rand)
{
    uint64_t rand_int = rand->Next();
    int64_t key_rand;
    if (read_random_exp_range_ == 0) {
        key_rand = rand_int % flags_num;
    } else {
        const uint64_t kBigInt = static_cast<uint64_t>(1U) << 62;
        long double order = -static_cast<long double>(rand_int % kBigInt) / static_cast<long double>(kBigInt) * read_random_exp_range_;
        long double exp_ran = std::exp(order);
        uint64_t rand_num = static_cast<int64_t>(exp_ran * static_cast<long double>(flags_num));
        // Map to a different number to avoid locality.
        const uint64_t kBigPrime = 0x5bd1e995;
        // Overflow is like %(2^64). Will have little impact of results.
        key_rand = static_cast<int64_t>((rand_num * kBigPrime) % flags_num);
    }
    return key_rand;
}

SingleQuery Benchmark::MixGraph()
{

    std::unique_ptr<const char[]> key_guard;
    Slice key = AllocateKey(&key_guard);

    int64_t ini_rand, rand_v, key_rand, key_seed;
    ini_rand = GetRandomKey(&this->rand);
    rand_v = ini_rand % flags_num;
    double u = static_cast<double>(rand_v) / flags_num;

    // Generate the keyID based on the key hotness and prefix hotness
    if (use_prefix_modeling) {
        key_rand = gen_exp.DistGetKeyID(ini_rand, flags_key_dist_a, flags_key_dist_b);
    } else {
        if (use_random_modeling) {
            key_rand = ini_rand;
        } else {
            key_seed = PowerCdfInversion(u, flags_key_dist_a, flags_key_dist_b);
            Random64 rand(key_seed);
            // use int64_t or uint64_t is not certain
            key_rand = static_cast<uint64_t>(rand.Next()) % flags_num;
        }
    }
#if OUTPUT_DEBUG
    // outpupt key_seed for debugging:
    std::ofstream ofile("key.txt", std::ios::app);
    if (!ofile) {
        fprintf(stderr, "can not open file\n");
    }
    ofile << key_rand << "\n";
    ofile.close();
#endif

    GenerateKeyFromInt(key_rand, flags_num, &key);
    //printf("keyrand : %ld\n", key_rand);
    int query_type = query.GetType(rand_v);

    // change the qps ... omitted

    // start query:
    SingleQuery ret;
    if (query_type == 0) {
        // Get query
        ret.valid = true;
        ret.type = Get;
        //ret.key  = Slice(key.data_, key.size_);
        ret.key = std::string(key.data_, key.size_);
        ret.key_size = key.size_;
        return ret;
    } else if (query_type == 1) {
        // Put query
        int64_t val_size;
        if (use_static_value_size) {
            val_size = flags_avg_value_size;
        } else {
            val_size = ParetoCdfInversion(
                u, flags_value_theta, flags_value_k, flags_value_sigma);
        }
        if (val_size < 0) {
            val_size = 10;
        } else if (val_size > value_max) {
            val_size = val_size % value_max;
        }
        ret.valid = true;
        ret.type = Put;
        /*
        ret.key = key;
        ret.val = gen.Generate(static_cast<unsigned int>(val_size));
        */
        ret.key = std::string(key.data_, key.size_);
        Slice tmp = gen.Generate(static_cast<unsigned int>(val_size));
        ret.val = std::string(tmp.data_, tmp.size_);
        ret.key_size = key.size_;
        ret.val_size = tmp.size_;
        return ret;
    } else if (query_type == 2) {
        ret.valid = true;
        ret.type = SeekForPrev;
        int64_t scan_len = ParetoCdfInversion(u, flags_iter_theta, flags_iter_k,
                               flags_iter_sigma)
            % flags_mix_max_scan_len;
        ret.key = std::string(key.data_, key.size_);
        ret.key_size = key.size_;
        ret.scan_len = scan_len;
        return ret;
    }
}

SingleQuery Benchmark::ReadRandom()
{
    static int num_keys = 0;
    int64_t key_rand = GetRandomKey(&this->rand);
    std::unique_ptr<const char[]> key_guard;
    Slice key = AllocateKey(&key_guard);

    if (entries_per_batch > 1 && flags_multiread_stride) {
        if (++num_keys == entries_per_batch) {
            num_keys = 0;
            key_rand = GetRandomKey(&this->rand);
            if ((key_rand + (entries_per_batch - 1) * flags_multiread_stride) >= flags_num) {
                key_rand = flags_num - entries_per_batch * flags_multiread_stride;
            }
        } else {
            key_rand += flags_multiread_stride;
        }
    } else {
        key_rand = GetRandomKey(&this->rand);
    }
    GenerateKeyFromInt(key_rand, flags_num, &key);
    SingleQuery ret;
    ret.type = Get;
    ret.key = std::string(key.data_, key.size_);
    ret.key_size = key.size_;
    return ret;
}

// This Benchmark generates Get and Put queries in specific ratio
// Keys(Both Get and Put) are generated randomly, as the same in ReadRandom
// Value size are 100 by default
SingleQuery Benchmark::MixRandom()
{
    static int num_keys = 0;
    // get random value for generating query type
    int64_t ini_rand, rand_v;
    ini_rand = GetRandomKey(&this->rand);
    rand_v = ini_rand % flags_num;

    int query_type = query.GetType(rand_v);

    // generate random key for Get or Put
    int64_t key_rand = GetRandomKey(&this->rand);
    std::unique_ptr<const char[]> key_guard;
    Slice key = AllocateKey(&key_guard);

    if (entries_per_batch > 1 && flags_multiread_stride) {
        if (++num_keys == entries_per_batch) {
            num_keys = 0;
            key_rand = GetRandomKey(&this->rand);
            if ((key_rand + (entries_per_batch - 1) * flags_multiread_stride) >= flags_num) {
                key_rand = flags_num - entries_per_batch * flags_multiread_stride;
            }
        } else {
            key_rand += flags_multiread_stride;
        }
    } else {
        key_rand = GetRandomKey(&this->rand);
    }

    GenerateKeyFromInt(key_rand, flags_num, &key);

    SingleQuery ret;

    if (query_type == 0) {
        ret.type = Get;
        ret.key = std::string(key.data_, key.size_);
        ret.key_size = key.size_;
        return ret;
    } else if (query_type == 1) {
        ret.type = Put;
        ret.key = std::string(key.data_, key.size_);
        // Generate random data
        Slice tmp = gen.Generate(static_cast<unsigned int>(value_size));
        ret.val = std::string(tmp.data_, tmp.size_);
        ret.key_size = key.size_;
        ret.val_size = tmp.size_;
        return ret;
    }
}

Options rocksdb_predef_bench_options(std::string bench_type)
{
    if (bench_type == "ZippyDB") {
        return Options(/*bench_name=*/"MixGraph", /*flags_num=*/50000000, /*read_random_exp_range=*/0,
            /*keys_per_prefix=*/0, /*key_size=*/16, /*rand_seed=*/1000, /*flags_ops_between_duration_checks=*/1000,
            /*entries_per_batch=*/1, /*flags_multiread_stride=*/0,
            /*flags_value_size_min=*/100, /*flags_value_size_max=*/102400,
            /*flags_mix_max_scan_len=*/10000, /*flags_mix_max_value_size=*/1024, /*flags_avg_value_size=*/16,
            /*flags_compression_ratio=*/0.5,
            /*flags_mix_get_ratio=*/0.85, /*flags_mix_put_ratio=*/0.14, /*flags_mix_seek_ratio=*/0.01,
            /*flags_keyrange_dist_a=*/14.18, /*flags_keyrange_dist_b=*/-2.917,
            /*flags_keyrange_dist_c=*/0.0164, /*flags_keyrange_dist_d=*/-0.08082,
            /*flags_key_dist_a=*/0.0, /*flags_key_dist_b=*/0.0,
            /*flags_value_theta=*/0.0, /*flags_value_k=*/0.2615, /*flags_value_sigma=*/25.45,
            /*flags_iter_theta=*/0.0, /*flags_iter_k=*/2.517, /*flags_iter_sigma=*/14.236,
            /*keyrange_num=*/30, /*monitor_range_num=*/30, /*use_static_value_size=*/1);
    } else if (bench_type == "UDB") {
        return Options(/*bench_name=*/"MixGraph", /*flags_num=*/50000000, /*read_random_exp_range=*/0,
            /*keys_per_prefix=*/0, /*key_size=*/27, /*rand_seed=*/1000, /*flags_ops_between_duration_checks=*/1000,
            /*entries_per_batch=*/1, /*flags_multiread_stride=*/0,
            /*flags_value_size_min=*/100, /*flags_value_size_max=*/102400,
            /*flags_mix_max_scan_len=*/10000, /*flags_mix_max_value_size=*/1024, /*flags_avg_value_size=*/256,
            /*flags_compression_ratio=*/0.5,
            /*flags_mix_get_ratio=*/0.84, /*flags_mix_put_ratio=*/0.13, /*flags_mix_seek_ratio=*/0.03,
            /*flags_keyrange_dist_a=*/0.0, /*flags_keyrange_dist_b=*/0.0,
            /*flags_keyrange_dist_c=*/0.0, /*flags_keyrange_dist_d=*/0.0,
            /*flags_key_dist_a=*/0.001636, /*flags_key_dist_b=*/-0.7094,
            /*flags_value_theta=*/0.0, /*flags_value_k=*/0.923, /*flags_value_sigma=*/226.409,
            /*flags_iter_theta=*/0.0, /*flags_iter_k=*/0.0819, /*flags_iter_sigma=*/1.747,
            /*keyrange_num=*/0, /*monitor_range_num=*/0, /*use_static_value_size=*/1);
    } else if (bench_type == "ReadRandom") {
        return Options(/*bench_name=*/"ReadRandom", /*flags_num=*/1000000, /*read_random_exp_range=*/0,
            /*keys_per_prefix=*/0, /*key_size=*/16, /*rand_seed=*/1000, /*flags_ops_between_duration_checks=*/1000,
            /*entries_per_batch=*/1, /*flags_multiread_stride=*/0,
            /*flags_value_size_min=*/100, /*flags_value_size_max=*/102400,
            /*flags_mix_max_scan_len=*/10000, /*flags_mix_max_value_size=*/1024, /*flags_avg_value_size=*/256,
            /*flags_compression_ratio=*/0.5,
            /*flags_mix_get_ratio=*/0.84, /*flags_mix_put_ratio=*/0.13, /*flags_mix_seek_ratio=*/0.03,
            /*flags_keyrange_dist_a=*/0.0, /*flags_keyrange_dist_b=*/0.0,
            /*flags_keyrange_dist_c=*/0.0, /*flags_keyrange_dist_d=*/0.0,
            /*flags_key_dist_a=*/0.0, /*flags_key_dist_b=*/0.0,
            /*flags_value_theta=*/0.0, /*flags_value_k=*/0.0, /*flags_value_sigma=*/0.0,
            /*flags_iter_theta=*/0.0, /*flags_iter_k=*/0.0, /*flags_iter_sigma=*/0.0,
            /*keyrange_num=*/0, /*monitor_range_num=*/30, /*use_static_value_size=*/1);
    } else if (bench_type == "MixRandom") {
        return Options(/*bench_name=*/"MixRandom", /*flags_num=*/1000000, /*read_random_exp_range=*/0,
            /*keys_per_prefix=*/0, /*key_size=*/16, /*rand_seed=*/1000, /*flags_ops_between_duration_checks=*/1000,
            /*entries_per_batch=*/1, /*flags_multiread_stride=*/0,
            /*flags_value_size_min=*/100, /*flags_value_size_max=*/102400,
            /*flags_mix_max_scan_len=*/10000, /*flags_mix_max_value_size=*/1024, /*flags_avg_value_size=*/256,
            /*flags_compression_ratio=*/0.5,
            /*flags_mix_get_ratio=*/0.8, /*flags_mix_put_ratio=*/0.2, /*flags_mix_seek_ratio=*/0.0,
            /*flags_keyrange_dist_a=*/0.0, /*flags_keyrange_dist_b=*/0.0,
            /*flags_keyrange_dist_c=*/0.0, /*flags_keyrange_dist_d=*/0.0,
            /*flags_key_dist_a=*/0.0, /*flags_key_dist_b=*/0.0,
            /*flags_value_theta=*/0.0, /*flags_value_k=*/0.0, /*flags_value_sigma=*/0.0,
            /*flags_iter_theta=*/0.0, /*flags_iter_k=*/0.0, /*flags_iter_sigma=*/0.0,
            /*keyrange_num=*/0, /*monitor_range_num=*/30, /*use_static_value_size=*/1);
    } else if (bench_type == "AllRandom") {
        return Options(/*bench_name=*/"MixGraph", /*flags_num=*/50000000, /*read_random_exp_range=*/0,
            /*keys_per_prefix=*/0, /*key_size=*/16, /*rand_seed=*/1000, /*flags_ops_between_duration_checks=*/1000,
            /*entries_per_batch=*/1, /*flags_multiread_stride=*/0,
            /*flags_value_size_min=*/100, /*flags_value_size_max=*/102400,
            /*flags_mix_max_scan_len=*/10000, /*flags_mix_max_value_size=*/1024, /*flags_avg_value_size=*/256,
            /*flags_compression_ratio=*/0.5,
            /*flags_mix_get_ratio=*/0.83, /*flags_mix_put_ratio=*/0.14, /*flags_mix_seek_ratio=*/0.03,
            /*flags_keyrange_dist_a=*/0.0, /*flags_keyrange_dist_b=*/0.0,
            /*flags_keyrange_dist_c=*/0.0, /*flags_keyrange_dist_d=*/0.0,
            /*flags_key_dist_a=*/0.0, /*flags_key_dist_b=*/0.0,
            /*flags_value_theta=*/0.0, /*flags_value_k=*/0.2615, /*flags_value_sigma=*/25.45,
            /*flags_iter_theta=*/0.0, /*flags_iter_k=*/2.517, /*flags_iter_sigma=*/14.236,
            /*keyrange_num=*/1, /*monitor_range_num=*/30, /*use_static_value_size=*/1);
    } else if (bench_type == "AllDist") {
        return Options(/*bench_name=*/"MixGraph", /*flags_num=*/50000000, /*read_random_exp_range=*/0,
            /*keys_per_prefix=*/0, /*key_size=*/16, /*rand_seed=*/1000, /*flags_ops_between_duration_checks=*/1000,
            /*entries_per_batch=*/1, /*flags_multiread_stride=*/0,
            /*flags_value_size_min=*/100, /*flags_value_size_max=*/102400,
            /*flags_mix_max_scan_len=*/10000, /*flags_mix_max_value_size=*/1024, /*flags_avg_value_size=*/256,
            /*flags_compression_ratio=*/0.5,
            /*flags_mix_get_ratio=*/0.83, /*flags_mix_put_ratio=*/0.14, /*flags_mix_seek_ratio=*/0.03,
            /*flags_keyrange_dist_a=*/0.0, /*flags_keyrange_dist_b=*/0.0,
            /*flags_keyrange_dist_c=*/0.0, /*flags_keyrange_dist_d=*/0.0,
            /*flags_key_dist_a=*/0.002312, /*flags_key_dist_b=*/0.3467,
            /*flags_value_theta=*/0.0, /*flags_value_k=*/0.2615, /*flags_value_sigma=*/25.45,
            /*flags_iter_theta=*/0.0, /*flags_iter_k=*/2.517, /*flags_iter_sigma=*/14.236,
            /*keyrange_num=*/1, /*monitor_range_num=*/30, /*use_static_value_size=*/1);
    } else if (bench_type == "PrefixDist") {
        return Options(/*bench_name=*/"MixGraph", /*flags_num=*/50000000, /*read_random_exp_range=*/0,
            /*keys_per_prefix=*/0, /*key_size=*/16, /*rand_seed=*/1000, /*flags_ops_between_duration_checks=*/1000,
            /*entries_per_batch=*/1, /*flags_multiread_stride=*/0,
            /*flags_value_size_min=*/100, /*flags_value_size_max=*/102400,
            /*flags_mix_max_scan_len=*/10000, /*flags_mix_max_value_size=*/1024, /*flags_avg_value_size=*/256,
            /*flags_compression_ratio=*/0.5,
            /*flags_mix_get_ratio=*/0.83, /*flags_mix_put_ratio=*/0.14, /*flags_mix_seek_ratio=*/0.03,
            /*flags_keyrange_dist_a=*/14.18, /*flags_keyrange_dist_b=*/-2.917,
            /*flags_keyrange_dist_c=*/0.0164, /*flags_keyrange_dist_d=*/-0.08082,
            /*flags_key_dist_a=*/0.002312, /*flags_key_dist_b=*/0.3467,
            /*flags_value_theta=*/0.0, /*flags_value_k=*/0.2615, /*flags_value_sigma=*/25.45,
            /*flags_iter_theta=*/0.0, /*flags_iter_k=*/2.517, /*flags_iter_sigma=*/14.236,
            /*keyrange_num=*/30, /*monitor_range_num=*/30, /*use_static_value_size=*/1);
    }
}

Benchmark rocksdb_bench_tool(std::string bench_type)
{
    Options bench_options = rocksdb_predef_bench_options(bench_type);
    return Benchmark(bench_options);
}
}
