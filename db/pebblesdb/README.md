# Compile PebblesDB (Version 1.0)
unzip pebblesdb.zip
cd pebblesdb/src
autoreconf -i
./configure
make
make install
cp -r /usr/local/lib/pebblesdb.a /home/XXXX