/*
 * @Author: your name
 * @Date: 2021-03-18 11:55:19
 * @LastEditTime: 2021-06-09 15:06:29
 * @LastEditors: Han Shukai
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/db/matrixkv/db.cc
 */
#include "db.h"
#include "matrixkv.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new MatrixKV(options);
}

void DB::Close(DB* dbptr)
{
    delete dbptr;
}