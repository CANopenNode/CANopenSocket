#!/usr/bin/env ../libs/bats-core/bin/bats
load '../libs/bats-support/load.bash'
load '../libs/bats-assert/load.bash'

setup_file() {
    export HB=100 #Heartbeat time for testing in milliseconds
    export OK=$'OK\r'

    export D1=01 # Device 1 (with command interface)
    export D2=04 # Device 2
    export D3=63 # Device 3
    export D4=7F # Device 4

    export B1=81 # Device 1 + 0x80
    export B2=84 # Device 2 + 0x80
    export B3=E3 # Device 3 + 0x80
    export B4=FF # Device 4 + 0x80

    #export cocomm_host=
    #export cocomm_port=60000
    export cocomm_socket=/tmp/CO_command_socket_test
    export cocomm_flat=1
    export cocomm_candump=can_test
    export cocomm_candump_count=10
    export cocomm_candump_timeout=$(( $HB * 2 ))
}

@test 'NMT: all preoperational' {
    # cocomm must exit with success (return value is 0 (and output is "[1] OK"))
    # and must receive at least one CAN message (option -n1) in time interval.
    assert ./cocomm -n1 "0 preoperational"
}

@test 'SDO: read heartbeat D1' {
    # cocomm must exit with success.
    # There is no can message, because SDO of master device works on itself.
    assert ./cocomm -n0 "0x$D1 r 0x1017 0 i16"
}

@test 'SDO: read heartbeat D2' {
    # cocomm must exit with success and must receive two CAN messages.
    assert ./cocomm -n2 "0x$D2 r 0x1017 0 i16"
}

@test 'SDO: read heartbeat D3' {
    assert ./cocomm -n2 "0x$D3 r 0x1017 0 i16"
}

@test 'SDO: read heartbeat D4' {
    assert ./cocomm -n2 "0x$D4 r 0x1017 0 i16"
}

@test 'SDO: disable heartbeats' {
    # Run cocomm with four commands, it must exit with success (response from
    # each command is success, none of responses is "ERROR").
    run ./cocomm -n6 "[1] 0x$D1 w 0x1017 0 i16 0" \
                     "[2] 0x$D2 w 0x1017 0 i16 0" \
                     "[3] 0x$D3 w 0x1017 0 i16 0" \
                     "[4] 0x$D4 w 0x1017 0 i16 0"
    # run must exit with success (none of command responses is "ERROR").
    assert_success
    # run output must match the string - exact match including newlines and '\r'
    assert_output "[1] $OK
[2] $OK
[3] $OK
[4] $OK
6$D2#2B17100000000000
5$B2#6017100000000000
6$D3#2B17100000000000
5$B3#6017100000000000
6$D4#2B17100000000000
5$B4#6017100000000000"
}

@test 'candump: no communication, 2sec' {
    sleep 0.2
    # cocomm must exit with error (timeout - no message within 2000 ms)
    refute ./cocomm -n1 -T2000 "0x$D1 w 0x1017 0 i16 0"
}

@test 'Heartbeats: all present' {
    assert ./cocomm -n2 "[1] 0x$D1 w 0x1017 0 i16 $HB"
    assert ./cocomm -n0 "[2] 0x$D1 w 0x1017 0 i16 0"
    assert ./cocomm -n4 "[3] 0x$D2 w 0x1017 0 i16 $HB"
    assert ./cocomm -n2 "[4] 0x$D2 w 0x1017 0 i16 0"
    assert ./cocomm -n4 "[5] 0x$D3 w 0x1017 0 i16 $HB"
    assert ./cocomm -n2 "[6] 0x$D3 w 0x1017 0 i16 0"
    assert ./cocomm -n4 "[7] 0x$D4 w 0x1017 0 i16 $HB"
    assert ./cocomm -n2 "[8] 0x$D4 w 0x1017 0 i16 0"
}

@test 'NMT: reset communication - one' {
    run ./cocomm -n2 "[1] 0x$D1 reset comm"
    assert_success
    assert_output "[1] $OK
000#82$D1
7$D1#00"
    run ./cocomm -n2 "[2] 0x$D2 reset comm"
    assert_success
    assert_output "[2] $OK
000#82$D2
7$D2#00"
    run ./cocomm -n2 "[3] 0x$D3 reset comm"
    assert_success
    assert_output "[3] $OK
000#82$D3
7$D3#00"
    run ./cocomm -n2 "[4] 0x$D4 reset comm"
    assert_success
    assert_output "[4] $OK
000#82$D4
7$D4#00"
}

@test 'NMT: reset communication - two' {
    run ./cocomm -n2 "[1] 0x$D1 reset comm"
    assert_success
    assert_output "[1] $OK
000#82$D1
7$D1#00"
    run ./cocomm -n2 "[2] 0x$D2 reset comm"
    assert_success
    assert_output "[2] $OK
000#82$D2
7$D2#00"
    run ./cocomm -n2 "[3] 0x$D3 reset comm"
    assert_success
    assert_output "[3] $OK
000#82$D3
7$D3#00"
    run ./cocomm -n2 "[4] 0x$D4 reset comm"
    assert_success
    assert_output "[4] $OK
000#82$D4
7$D4#00"
}

@test 'NMT: reset communication - three' {
    run ./cocomm -n2 "[1] 0x$D1 reset comm"
    assert_success
    assert_output "[1] $OK
000#82$D1
7$D1#00"
    run ./cocomm -n2 "[2] 0x$D2 reset comm"
    assert_success
    assert_output "[2] $OK
000#82$D2
7$D2#00"
    run ./cocomm -n2 "[3] 0x$D3 reset comm"
    assert_success
    assert_output "[3] $OK
000#82$D3
7$D3#00"
    run ./cocomm -n2 "[4] 0x$D4 reset comm"
    assert_success
    assert_output "[4] $OK
000#82$D4
7$D4#00"
}

@test 'all preoperational' {
    assert ./cocomm -n1 "0 preoperational"
}

@test 'SDO: disable all PDOs on D1' {
    run ./cocomm -n0 "[11] 0x$D1 w 0x1400 1 u32 0x800002$D1" \
                     "[12] 0x$D1 w 0x1401 1 u32 0x800003$D1" \
                     "[13] 0x$D1 w 0x1402 1 u32 0x800004$D1" \
                     "[14] 0x$D1 w 0x1403 1 u32 0x800005$D1" \
                     "[15] 0x$D1 w 0x1800 1 u32 0x800001$B1" \
                     "[16] 0x$D1 w 0x1801 1 u32 0x800002$B1" \
                     "[17] 0x$D1 w 0x1802 1 u32 0x800003$B1" \
                     "[18] 0x$D1 w 0x1803 1 u32 0x800004$B1"
    assert_success
    assert_output "[11] $OK
[12] $OK
[13] $OK
[14] $OK
[15] $OK
[16] $OK
[17] $OK
[18] $OK"
}
@test 'SDO: disable all PDOs on D2' {
    run ./cocomm -n16 "[21] 0x$D2 w 0x1400 1 u32 0x800002$D2" \
                      "[22] 0x$D2 w 0x1401 1 u32 0x800003$D2" \
                      "[23] 0x$D2 w 0x1402 1 u32 0x800004$D2" \
                      "[24] 0x$D2 w 0x1403 1 u32 0x800005$D2" \
                      "[25] 0x$D2 w 0x1800 1 u32 0x800001$B2" \
                      "[26] 0x$D2 w 0x1801 1 u32 0x800002$B2" \
                      "[27] 0x$D2 w 0x1802 1 u32 0x800003$B2" \
                      "[28] 0x$D2 w 0x1803 1 u32 0x800004$B2"
    assert_success
    assert_output "[21] $OK
[22] $OK
[23] $OK
[24] $OK
[25] $OK
[26] $OK
[27] $OK
[28] $OK
6$D2#23001401"$D2"020080
5$B2#6000140100000000
6$D2#23011401"$D2"030080
5$B2#6001140100000000
6$D2#23021401"$D2"040080
5$B2#6002140100000000
6$D2#23031401"$D2"050080
5$B2#6003140100000000
6$D2#23001801"$B2"010080
5$B2#6000180100000000
6$D2#23011801"$B2"020080
5$B2#6001180100000000
6$D2#23021801"$B2"030080
5$B2#6002180100000000
6$D2#23031801"$B2"040080
5$B2#6003180100000000"
}
@test 'SDO: disable all PDOs on D3' {
    run ./cocomm -n16 "[31] 0x$D3 w 0x1400 1 u32 0x800002$D3" \
                      "[32] 0x$D3 w 0x1401 1 u32 0x800003$D3" \
                      "[33] 0x$D3 w 0x1402 1 u32 0x800004$D3" \
                      "[34] 0x$D3 w 0x1403 1 u32 0x800005$D3" \
                      "[35] 0x$D3 w 0x1800 1 u32 0x800001$B3" \
                      "[36] 0x$D3 w 0x1801 1 u32 0x800002$B3" \
                      "[37] 0x$D3 w 0x1802 1 u32 0x800003$B3" \
                      "[38] 0x$D3 w 0x1803 1 u32 0x800004$B3"
    assert_success
    assert_output "[31] $OK
[32] $OK
[33] $OK
[34] $OK
[35] $OK
[36] $OK
[37] $OK
[38] $OK
6$D3#23001401"$D3"020080
5$B3#6000140100000000
6$D3#23011401"$D3"030080
5$B3#6001140100000000
6$D3#23021401"$D3"040080
5$B3#6002140100000000
6$D3#23031401"$D3"050080
5$B3#6003140100000000
6$D3#23001801"$B3"010080
5$B3#6000180100000000
6$D3#23011801"$B3"020080
5$B3#6001180100000000
6$D3#23021801"$B3"030080
5$B3#6002180100000000
6$D3#23031801"$B3"040080
5$B3#6003180100000000"
}
@test 'SDO: disable all PDOs on D4' {
    run ./cocomm -n16 "[41] 0x$D4 w 0x1400 1 u32 0x800002$D4" \
                      "[42] 0x$D4 w 0x1401 1 u32 0x800003$D4" \
                      "[43] 0x$D4 w 0x1402 1 u32 0x800004$D4" \
                      "[44] 0x$D4 w 0x1403 1 u32 0x800005$D4" \
                      "[45] 0x$D4 w 0x1800 1 u32 0x800001$B4" \
                      "[46] 0x$D4 w 0x1801 1 u32 0x800002$B4" \
                      "[47] 0x$D4 w 0x1802 1 u32 0x800003$B4" \
                      "[48] 0x$D4 w 0x1803 1 u32 0x800004$B4"
    assert_success
    assert_output "[41] $OK
[42] $OK
[43] $OK
[44] $OK
[45] $OK
[46] $OK
[47] $OK
[48] $OK
6$D4#23001401"$D4"020080
5$B4#6000140100000000
6$D4#23011401"$D4"030080
5$B4#6001140100000000
6$D4#23021401"$D4"040080
5$B4#6002140100000000
6$D4#23031401"$D4"050080
5$B4#6003140100000000
6$D4#23001801"$B4"010080
5$B4#6000180100000000
6$D4#23011801"$B4"020080
5$B4#6001180100000000
6$D4#23021801"$B4"030080
5$B4#6002180100000000
6$D4#23031801"$B4"040080
5$B4#6003180100000000"
}

@test 'NMT: pre-operational D1' {
    run ./cocomm -n2 "[1] 0x$D1 w 0x1017 0 i16 $HB"
    assert_success
    assert_output "[1] $OK
7$D1#7F
7$D1#7F"
}
@test 'NMT: operational D1' {
    run ./cocomm -n3 "[1] 0x$D1 start"
    assert ./cocomm -n0 "0x$D1 w 0x1017 0 i16 0"
    assert_success
    assert_output "[1] $OK
000#01$D1
7$D1#05
7$D1#05"
}

@test 'NMT: pre-operational D2' {
    run ./cocomm -n4 "[1] 0x$D2 w 0x1017 0 i16 $HB"
    assert_success
    assert_output "[1] $OK
6$D2#2B17100064000000
5$B2#6017100000000000
7$D2#7F
7$D2#7F"
}
@test 'NMT: operational D2' {
    run ./cocomm -n3 "[1] 0x$D2 start"
    assert ./cocomm -n2 "0x$D2 w 0x1017 0 i16 0"
    assert_success
    assert_output "[1] $OK
000#01$D2
7$D2#05
7$D2#05"
}

@test 'NMT: pre-operational D3' {
    run ./cocomm -n4 "[1] 0x$D3 w 0x1017 0 i16 $HB"
    assert_success
    assert_output "[1] $OK
6$D3#2B17100064000000
5$B3#6017100000000000
7$D3#7F
7$D3#7F"
}
@test 'NMT: operational D3' {
    run ./cocomm -n3 "[1] 0x$D3 start"
    assert ./cocomm -n2 "0x$D3 w 0x1017 0 i16 0"
    assert_success
    assert_output "[1] $OK
000#01$D3
7$D3#05
7$D3#05"
}

@test 'NMT: pre-operational D4' {
    run ./cocomm -n4 "[1] 0x$D4 w 0x1017 0 i16 $HB"
    assert_success
    assert_output "[1] $OK
6$D4#2B17100064000000
5$B4#6017100000000000
7$D4#7F
7$D4#7F"
}
@test 'NMT: operational D4' {
    run ./cocomm -n3 "[1] 0x$D4 start"
    assert ./cocomm -n2 "0x$D4 w 0x1017 0 i16 0"
    assert_success
    assert_output "[1] $OK
000#01$D4
7$D4#05
7$D4#05"
}

@test 'finish: NMT all preoperational' {
    assert ./cocomm -n1 "0 preoperational"
}
