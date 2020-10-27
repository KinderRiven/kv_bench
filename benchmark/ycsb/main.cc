#include "db.h"
#include "header.h"
#include "options.h"
#include "workload_generator.h"
#include "ycsb.h"


int main(int argc, char* argv[])
{
    kv_benchmark::DB* _db;
    kv_benchmark::Options _options;
    kv_benchmark::YCSB* _benchmarks[32];

    // DB opation
    char _ssd_path[128] = "/home/hanshukai/dir1";
    char _pmem_path[128] = "/home/pmem0";

    // workload generator
    for (int i = 0; i < argc; i++) {
        char junk;
        uint64_t n;
        if (strncmp(argv[i], "--nvm=", 6) == 0) {
            strcpy(_pmem_path, argv[i] + 6);
        } else if (strncmp(argv[i], "--ssd=", 6) == 0) {
            strcpy(_ssd_path, argv[i] + 6);
        } else {
            return 0;
        }
    }

    kv_benchmark::DB::Open(_options, &_db);
    kv_benchmark::WorkloadGenerator *_wamrup = new kv_benchmark::WorkloadGenerator(nullptr, _db, _benchmarks);

    return 0;
}
