#!/usr/bin/env ../libs/bats-core/bin/bats
load '../libs/bats-support/load.bash'
load '../libs/bats-assert/load.bash'

setup_file() {
    . ./config_bats.sh

    if [[ $binaryFileSize = "" ]] ; then
        export binaryFileSize=100000
    fi
    if [[ $binaryFileSegSize = "" ]] ; then
        export binaryFileSegSize=1000
    fi

    export sedStr='s/^\([0-9]*\).*/\1/'
}

teardown_file() {
    cat binaryFileReport >&3
    rm binaryFile*
    run rm ../../examples/basicDevice/fileWrittenByDomain
}

@test 'Segmented transfer - set filename for upload' {
    assert $cocomm -n0 "set sdo_block 0"
    run $cocomm -n8 "[1] 0x$D2 w 0x2120 11 vs fileWrittenByDomain"
    assert_success
    assert_output "[1] $OK
6$D2#2120210B13000000
5$B2#6020210B00000000
6$D2#0066696C65577269
5$B2#2000000000000000
6$D2#107474656E427944
5$B2#3000000000000000
6$D2#056F6D61696E0000
5$B2#2000000000000000"
}

@test "Block transfer, create $binaryFileSize bytes long random binaryFile" {
    assert openssl rand $binaryFileSize > binaryFile
}

@test 'Block download binaryFile' {
    assert $cocomm -n0 "set sdo_block 1"
    time_begin=$(date +%s.%N)
    assert base64 -w0 binaryFile | $cocomm -n0 -i "[1] 0x$D2 w 0x2120 10 d"
    time_end=$(date +%s.%N)
    time_diff=$(echo "($time_end - $time_begin) * 1000" | bc | sed $sedStr)
    echo -n "# $binaryFileSize bytes block down/upload: $time_diff/" > binaryFileReport
}

@test 'Block upload to binaryFileReceived - 2x' {
    assert $cocomm -n0 "set sdo_block 1"
    time_begin=$(date +%s.%N)
    assert $cocomm -n0 -odata "[1] 0x$D2 r 0x2120 10 d" | base64 -d > binaryFileReceived1
    assert $cocomm -n0 -odata "[1] 0x$D2 r 0x2120 10 d" | base64 -d > binaryFileReceived2
    time_end=$(date +%s.%N)
    time_diff=$(echo "($time_end - $time_begin) * 1000 / 2" | bc | sed $sedStr)
    echo "$time_diff milliseconds." >> binaryFileReport
}

@test 'Block transfer, files equal' {
    assert cmp binaryFile binaryFileReceived1
    assert cmp binaryFile binaryFileReceived2
}

@test "Segmented transfer, create $binaryFileSegSize bytes long random binaryFileSeg" {
    assert openssl rand $binaryFileSegSize > binaryFileSeg
}

@test 'Segmented download binaryFileSeg' {
    assert $cocomm -n0 "set sdo_block 0"
    time_begin=$(date +%s.%N)
    assert base64 -w0 binaryFileSeg | $cocomm -n0 -i "[1] 0x$D2 w 0x2120 10 d"
    time_end=$(date +%s.%N)
    time_diff=$(echo "($time_end - $time_begin) * 1000" | bc | sed $sedStr)
    echo -n "# $binaryFileSegSize bytes segmented down/upload: $time_diff/" >> binaryFileReport
}

@test 'Segmented upload to binaryFileSegReceived' {
    assert $cocomm -n0 "set sdo_block 0"
    time_begin=$(date +%s.%N)
    assert $cocomm -n0 -odata "[1] 0x$D2 r 0x2120 10 d" | base64 -d > binaryFileSegReceived
    time_end=$(date +%s.%N)
    time_diff=$(echo "($time_end - $time_begin) * 1000" | bc | sed $sedStr)
    echo "$time_diff milliseconds." >> binaryFileReport
}

@test 'Segmented transfer, files equal' {
    assert cmp binaryFileSeg binaryFileSegReceived
}
