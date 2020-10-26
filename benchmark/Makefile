SRC_LEVELDB := ../db/leveldb/leveldb.cc ../db/leveldb/db.cc

all: leveldb

leveldb:
	g++ -std=c++11 -I../include $(SRC_LEVELDB) test.cc -o leveldb_tester -L../third-party/leveldb -lleveldb -lpthread -lsnappy
