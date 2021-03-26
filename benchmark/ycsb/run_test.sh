###
 # @Author: your name
 # @Date: 2021-03-24 15:38:05
 # @LastEditTime: 2021-03-26 10:52:40
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /kv_bench/benchmark/run.sh
### 

APP1=./rocksdb_tester
$APP1 --btype=write --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/rocksdb_db --num_backend_thread=1 --num_thread=4 --dbsize=100 --psize=0.5

APP3=./matrixkv_tester
$APP3 --btype=write --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/matrixkv_db --num_backend_thread=1 --num_thread=4 --dbsize=100 --psize=0.5

APP2=./silk_tester
$APP2 --btype=write --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/silk_db --num_backend_thread=1 --num_thread=4 --dbsize=100 --psize=0.5