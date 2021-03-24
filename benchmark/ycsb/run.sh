###
 # @Author: your name
 # @Date: 2021-03-24 15:38:05
 # @LastEditTime: 2021-03-24 16:05:33
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /kv_bench/benchmark/run.sh
### 

APP := ./rocksdb_tester
APP := ./matrixkv_tester
APP := ./silk_tester

$APP --btype=warmup --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/db --num_thread=4 --dbsize=100 --psize=0.5