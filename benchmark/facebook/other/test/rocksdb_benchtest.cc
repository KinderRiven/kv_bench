#include "rocksdb_benchtest.h"
#define MULTITHREAD_TEST

void printSlice(const ROCKSDB_BENCHMARK_NAMESPACE::Slice* s)
{
    const char* start = s->data_;
    for (size_t i = 0; i < s->size(); ++i)
        printf("%c", *(start + i));
    printf("\n");
}

// test:
ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery retval_test()
{
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark a = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_bench_tool("ZippyDB");
    std::unique_ptr<const char[]> key_guard;
    ROCKSDB_BENCHMARK_NAMESPACE::Slice s = a.AllocateKey(&key_guard);
    a.GenerateKeyFromInt(1, 500, &s);
    ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery ret;
    ret.key = std::string(s.data_, s.size_);
    return ret;
}

std::string retval_test2()
{
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark a = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_bench_tool("ZippyDB");
    std::unique_ptr<const char[]> key_guard;
    ROCKSDB_BENCHMARK_NAMESPACE::Slice s = a.AllocateKey(&key_guard);
    a.GenerateKeyFromInt(1, 500, &s);
    std::string ret(s.data_, s.size_);
    return ret;
}

void printStringToInt(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) {
        printf("%d", static_cast<int>(s[i]));
    }
    printf("\n");
}

// multithread test:

void multithread_test(ROCKSDB_BENCHMARK_NAMESPACE::Benchmark* arg)
{
    std::thread::id id = std::this_thread::get_id();
    const int max_times = 42000000;
    int time = 0;
    while (time < max_times) {
        ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery query = arg->GetSingleQuery();
        ++time;
        if (time % 100000 == 0) {
            fprintf(stdout, "Already done: %d\n", time);
        }
    }
}

int main()
{
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark test
        = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_bench_tool("MixRandom");

    test.InitiateBenchmark();
    // ask for 4200000 times quries

    const int max_times = 42000000;
    int time = 0;

#ifdef MULTITHREAD_TEST
    std::thread t1(multithread_test, &test);
    std::thread t2(multithread_test, &test);
    t1.join();
    t2.join();
    test.StatusReport();
#endif

#ifdef FIT_MODEL_TEST

    //record key
    std::ofstream ofile("key.txt");
    std::ofstream ofile2("value_size.txt");
    if (!ofile || !ofile2) {
        std::cerr << "open file failed" << std::endl;
        exit(1);
    }
    while (time < max_times) {
        ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery query = test.GetSingleQuery();
        int64_t key = ConvertKeyToInt(query.key);
        ofile << key << "\n";
        if (query.type == ROCKSDB_BENCHMARK_NAMESPACE::Put) {
            size_t valsz = query.val.size();
            if (valsz > 0) {
                ofile2 << valsz << "\n";
            }
        }
        ++time;
        if (time % 100000 == 0) {
            printf("already done : %d\n", time);
        }
    }
    ofile.close();
    ofile2.close();
#endif

#ifdef RATIO_TEST
    int read = 0, write = 0, total = 0;
    while (time < max_times) {
        ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery query = test.GetSingleQuery();
        if (query.type == ROCKSDB_BENCHMARK_NAMESPACE::Get)
            ++read;
        else if (query.type == ROCKSDB_BENCHMARK_NAMESPACE::Put)
            ++write;
        ++total;
        printf("Get ratio: %.6lf, Put ratio: %.6lf\n",
            (double)read / total, (double)write / total);
    }
#endif

#ifdef STATUS_REPORT

    while (time < max_times) {
        ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery query = test.GetSingleQuery();
        ++time;
        if (time % 100000 == 0) {
            printf("already done : %d\n", time);
        }
    }
    test.StatusReport();
#endif
    return 0;
}