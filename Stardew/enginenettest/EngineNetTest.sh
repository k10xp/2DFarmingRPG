#!/bin/bash
../build/enginenettest/EngineNetTest -r s -s 127.0.0.1:666 -l i --output_file_path ./random_serverout.bin --server_timeout 10 &

# random file
head -c 512 < /dev/urandom > ./random.bin


../build/enginenettest/EngineNetTest -r c -s 127.0.0.1:666 -l i --input_file_path ./random.bin --output_file_path ./random_clientout.bin

sha1sum -b ./random.bin
sha1sum -b ./random_clientout.bin
sha1sum -b ./random_serverout.bin

echo "SHAs should match"
rm -f ./random.bin
rm -f ./random_clientout.bin
rm -f ./random_serverout.bin

# just in case
killall -s KILL EngineNetTest