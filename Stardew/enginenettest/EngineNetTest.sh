#!/bin/bash

#
# $1 : random file size
# $2 : random file name
# $3 : client output file name
# $4 : server output file
basic_test() {

    ../build/enginenettest/EngineNetTest -r s -s 127.0.0.1:666 -l i --output_file_path $4 --server_timeout 1 --lf ./server_process.log &

    # random file
    head -c $1 < /dev/urandom > $2

    ../build/enginenettest/EngineNetTest -r c -s 127.0.0.1:666 -l i --input_file_path $2 --output_file_path $3

    sleep 2s

    printf "\n\n\n\nSERVER LOG\n\n\n\n"

    cat ./server_process.log

    printf "\n\n\n\n"

    echo "SHAs should match"
    ORIG_SHA="$(sha1sum -b $2 | grep -E -o '^[^[:space:]]+' | uniq)"
    CLIENT_SHA="$(sha1sum -b $3 | grep -E -o '^[^[:space:]]+' | uniq)"
    SERVER_SHA="$(sha1sum -b $4 | grep -E -o '^[^[:space:]]+' | uniq)"
    sha1sum -b $2
    sha1sum -b $3
    sha1sum -b $4

    #cleanup
    rm -f $2
    rm -f $3
    rm -f $4
    rm -f ./server_process.log

    if [[ "$ORIG_SHA" == "$CLIENT_SHA" || "$ORIG_SHA" == "$SERVER_SHA" ]]; then
        echo "SHA's Match!"
        return 0
    else
        echo "SHA's DO NOT Match!"
        return 1
    fi
    return 0
}

# $1 : random file size
# $2 : random file name
# $3 : client output file name
# $4 : server output file

basic_test 512 ./random.bin ./client_out.bin ./server_out.bin

killall -s KILL EngineNetTest
