/*
 * @Author: your name
 * @Date: 2021-03-24 15:28:08
 * @LastEditTime: 2021-03-24 15:33:04
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/db/silk/db.cc
 */
#include "db.h"
#include "silk.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new SILKDB(options);
}