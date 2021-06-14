puts "This is a Ruby Script!";

bs = [64, 256, 1024, 4096, 16384]
db = ["leveldb", "rocksdb", "matrixkv", "pebblesdb"]

bs.each do |e_bs|
    puts e_bs
    db.each do |e_db|
        run = e_db + "_tester";
        puts run
    end
end