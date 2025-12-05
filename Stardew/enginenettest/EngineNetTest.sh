#!/bin/bash
../build/enginenettest/EngineNetTest -r s -s 127.0.0.1:666 -l i --output_file_path ./random_serverout.bin --server_timeout 1 --lf ./server_process.log &

# random file
head -c 512 < /dev/urandom > ./random.bin


../build/enginenettest/EngineNetTest -r c -s 127.0.0.1:666 -l i --input_file_path ./random.bin --output_file_path ./random_clientout.bin

sleep 2s

printf "\n\n\n\nSERVER LOG\n\n\n\n"

cat ./server_process.log


printf "\n\n\n\n"


echo "SHAs should match"
sha1sum -b ./random.bin
sha1sum -b ./random_clientout.bin
sha1sum -b ./random_serverout.bin

#cleanup
rm -f ./random.bin
rm -f ./random_clientout.bin
rm -f ./random_serverout.bin
rm -f ./server_process.log

killall -s KILL EngineNetTest
