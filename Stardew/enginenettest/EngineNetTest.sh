#!/bin/bash

NET_TEST_EXE_PATH="../build/enginenettest/EngineNetTest"

# $1 : output file path
# $2 : server timeout
# $3 : console log disabled
start_server() {
    local CMD=""
    if [[ $3 == "true" ]]; then
        CMD="$NET_TEST_EXE_PATH -r s -s 127.0.0.1:666 -l i --output_file_path $1 --server_timeout $2 --lf ./server_process.log --disable_console_log"
    else
        CMD="$NET_TEST_EXE_PATH -r s -s 127.0.0.1:666 -l i --output_file_path $1 --server_timeout $2 --lf ./server_process.log"
    fi
    $CMD
}

# $1 : size in bytes
# $2 : output f1le path 
random_file() {
    head -c $1 < /dev/urandom > $2
}

# $1 : input file path
# $2 : output file path
# $3 : disable console log
# $4 : log file 
start_client() {
    local CMD=""
    if [[ $3 == "true" ]]; then
        CMD="$NET_TEST_EXE_PATH -r c -s 127.0.0.1:666 -l i --input_file_path $1 --output_file_path $2 --lf $4 --disable_console_log"
    else
        CMD="$NET_TEST_EXE_PATH -r c -s 127.0.0.1:666 -l i --input_file_path $1 --output_file_path $2 --lf $4"
    fi
    $CMD
}

# kill any stray test processes
kill_test_processes() {
    killall -s KILL EngineNetTest &> /dev/null
}

#
# $1 : original file
# $2 : client output
# $3 : server output
compare_test_files() {
    local ORIG_SHA="$(sha1sum -b $1 | grep -E -o '^[^[:space:]]+' | uniq)"
    local CLIENT_SHA="$(sha1sum -b $2 | grep -E -o '^[^[:space:]]+' | uniq)"
    local SERVER_SHA="$(sha1sum -b $3 | grep -E -o '^[^[:space:]]+' | uniq)"

    printf "%s %s\n" $ORIG_SHA $1
    printf "%s %s\n" $CLIENT_SHA $2
    printf "%s %s\n" $SERVER_SHA $3
    if [[ "$ORIG_SHA" != "$CLIENT_SHA" || "$ORIG_SHA" != "$SERVER_SHA" ]]; then
        echo "SHA's DO NOT Match!"
        return 1
    else
        echo "SHA's Match!"
        return 0
    fi
}

#
# $1 : random file size
# $2 : random file name
# $3 : client output file name
# $4 : server output file
basic_test() {
    local RANDOM_FILE_SIZE=$1
    local RANDOM_FILE_PATH=$2
    local CLIENT_OUTPUT_FILE_PATH=$3
    local SERVER_OUTPUT_FILE_PATH=$4

    local FILES_TO_CLEANUP=( $RANDOM_FILE_PATH $CLIENT_OUTPUT_FILE_PATH $SERVER_OUTPUT_FILE_PATH "./client_process.log" "./server_process.log" )
    start_server \
        $SERVER_OUTPUT_FILE_PATH \
        1.5s \
        true \
        &

    # random file
    random_file \
        $RANDOM_FILE_SIZE \
        $RANDOM_FILE_PATH

    sleep 0.5s

    start_client \
        $RANDOM_FILE_PATH \
        $CLIENT_OUTPUT_FILE_PATH \
        true \
         ./client_process.log

    sleep 2s

    kill_test_processes

    printf "\n\n\n\nSERVER LOG\n\n\n\n"

    cat ./server_process.log

    printf "\n\n\n\nCLIENT LOG\n\n\n\n"

    cat ./client_process.log

    printf "\n\n\n\n"

    echo "SHAs should match"

    compare_test_files \
        $RANDOM_FILE_PATH \
        $CLIENT_OUTPUT_FILE_PATH \
        $SERVER_OUTPUT_FILE_PATH

    local COMPARISON_RESULT=$?

    #cleanup
    printf "Cleanup \n\n\n\n"
    for i in ${!FILES_TO_CLEANUP[@]};
    do
        local FILE=${FILES_TO_CLEANUP[$i]}
        echo "deleting $FILE"
        rm -f $FILE
    done

    printf "\n\n\n\n"

    if [[ $COMPARISON_RESULT == 1 ]]; then
        echo "Test failed!"
        return 1
    else
        echo "Test passed!"
        return 0
    fi
    return 0
}


basic_test 512 ./random.bin ./client_out.bin ./server_out.bin


