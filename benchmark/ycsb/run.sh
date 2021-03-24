###
 # @Author: your name
 # @Date: 2021-03-24 15:38:05
 # @LastEditTime: 2021-03-24 19:21:45
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /kv_bench/benchmark/run.sh
### 

APP1=./rocksdb_tester
$APP1 --btype=warmup --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/db --num_thread=4 --dbsize=100 --psize=0.5
$APP1 --btype=ycsb-a --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/db --num_thread=4 --dbsize=100 --psize=0.5
rm -rf /home/hanshukai/mount/4510/db

APP2=./silk_tester
$APP2 --btype=warmup --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/db --num_thread=4 --dbsize=100 --psize=0.5
$APP2 --btype=ycsb-a --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/db --num_thread=4 --dbsize=100 --psize=0.5
rm -rf /home/hanshukai/mount/4510/db

APP3=./matrixkv_tester
$APP3 --btype=warmup --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/db --num_thread=4 --dbsize=100 --psize=0.5
$APP3 --btype=ycsb-a --nvm=/home/pmem0 --ssd=/home/hanshukai/mount/4510/db --num_thread=4 --dbsize=100 --psize=0.5
rm -rf /home/hanshukai/mount/4510/db