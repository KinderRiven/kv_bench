make
echo "********RUNNING PFHT*************"
touch /home/pmem0/rocksdb_test/pfht
./db_bench --index=PFHT --opt_num=42000000 > test_result/PFHT.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/pfht
echo "********RUNNING FAST_FAIR*************"
touch /home/pmem0/rocksdb_test/fast_fair
./db_bench --index=FAST_FAIR --opt_num=42000000 > test_result/FAST_FAIR.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/fast_fair
echo "********RUNNING FPTREE*************"
touch /home/pmem0/rocksdb_test/fptree
./db_bench --index=FPTREE --opt_num=42000000 > test_result/FPTREE.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/fptree
echo "********RUNNING LEVEL_HASH*************"
touch /home/pmem0/rocksdb_test/level_hash
./db_bench --index=LEVEL_HASH --opt_num=42000000 > test_result/LEVEL_HASH.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/level_hash
echo "********RUNNING CCEH_LSB_HASH*************"
touch /home/pmem0/rocksdb_test/cceh_lsb
./db_bench --index=CCEH_LSB_HASH --opt_num=42000000 > test_result/CCEH_LSB_HASH.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/cceh_lsb
echo "********RUNNING HIKV*************"
touch /home/pmem0/rocksdb_test/hikv
./db_bench --index=HIKV --opt_num=42000000 > test_result/HIKV.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/hikv
echo "********RUNNING SKIPLIST*************"
touch /home/pmem0/rocksdb_test/skiplist
./db_bench --index=SKIPLIST --opt_num=42000000 > test_result/SKIPLIST.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/skiplist
echo "********RUNNING BDSKIPLIST*************"
touch /home/pmem0/rocksdb_test/bdskiplist
./db_bench --index=BDSKIPLIST --opt_num=42000000 > test_result/BDSKIPLIST.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/bdskiplist
echo "********RUNNING WBTREE*************"
touch /home/pmem0/rocksdb_test/wbtree
./db_bench --index=WBTREE --opt_num=42000000 > test_result/WBTREE.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/wbtree
echo "********RUNNING WORT*************"
touch /home/pmem0/rocksdb_test/wort
./db_bench --index=WORT --opt_num=42000000 > test_result/WORT.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/wort
echo "********RUNNING NORMAL_HASH*************"
touch /home/pmem0/rocksdb_test/normal_hash
./db_bench --index=NORMAL_HASH --opt_num=42000000 > test_result/NORMAL_HASH.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/normal_hash
echo "********RUNNING SHAOW_HASH*************"
touch /home/pmem0/rocksdb_test/shadow_hash
./db_bench --index=SHADOW_HASH --opt_num=42000000 > test_result/SHADOW_HASH.txt \
--pmem_file_path=/home/pmem0/rocksdb_test/shadow_hash

