puts "This is a Ruby Script!";

size = 100; # GB
bs = [64, 256, 1024, 4096, 16384] # Byte
db = ["leveldb", "rocksdb", "matrixkv", "pebblesdb"]

# puts "DB Size" + "[" + size + "GB]";

bs.each do |e_bs|
    puts e_bs
    db.each do |e_db|
        run = "./" + e_db + "_tester"
        system run
        puts run
    end
end