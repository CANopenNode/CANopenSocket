#!/usr/bin/env ../libs/bats-core/bin/bats
load '../libs/bats-support/load.bash'
load '../libs/bats-assert/load.bash'

setup_file() {
    . ./config_bats.sh
}

@test 'NMT: all preoperational' {
    # cocomm must exit with success (return value is 0 (and output is "[1] OK"))
    # and must receive at least one CAN message (option -n1) in time interval.
    assert $cocomm -n1 "0 preoperational"
}

@test 'SDO: read heartbeat D1' {
    # cocomm must exit with success.
    # There is no can message, because SDO of master device works on itself.
    assert $cocomm -n0 "0x$D1 r 0x1017 0 i16"
}

@test 'SDO: read heartbeat D2' {
    # cocomm must exit with success and must receive two CAN messages.
    assert $cocomm -n2 "0x$D2 r 0x1017 0 i16"
}

@test 'SDO: disable heartbeats' {
    # Run cocomm with four commands, it must exit with success (response from
    # each command is success, none of responses is "ERROR").
    run $cocomm -n2 "[1] 0x$D1 w 0x1017 0 i16 0" \
                     "[2] 0x$D2 w 0x1017 0 i16 0"
    # run must exit with success (none of command responses is "ERROR").
    assert_success
    # run output must match the string - exact match including newlines and '\r'
    assert_output "[1] $OK
[2] $OK
6$D2#2B17100000000000
5$B2#6017100000000000"
}

@test 'candump: no communication, 2sec' {
    sleep 0.2
    # cocomm must exit with error (timeout - no message within 2000 ms)
    refute $cocomm -n1 -T2000 "0x$D1 w 0x1017 0 i16 0"
}

@test 'Heartbeats: all present' {
    assert $cocomm -n2 "[1] 0x$D1 w 0x1017 0 i16 0x$HB"
    assert $cocomm -n0 "[2] 0x$D1 w 0x1017 0 i16 0"
    assert $cocomm -n4 "[3] 0x$D2 w 0x1017 0 i16 0x$HB"
    assert $cocomm -n2 "[4] 0x$D2 w 0x1017 0 i16 0"
}

@test 'NMT: reset communication - one' {
    run $cocomm -n2 "[1] 0x$D1 reset comm"
    assert_success
    assert_output "[1] $OK
000#82$D1
7$D1#00"
    run $cocomm -n2 "[2] 0x$D2 reset comm"
    assert_success
    assert_output "[2] $OK
000#82$D2
7$D2#00"
}

@test 'NMT: reset communication - two' {
    run $cocomm -n2 "[1] 0x$D1 reset comm"
    assert_success
    assert_output "[1] $OK
000#82$D1
7$D1#00"
    run $cocomm -n2 "[2] 0x$D2 reset comm"
    assert_success
    assert_output "[2] $OK
000#82$D2
7$D2#00"
}

@test 'NMT: reset communication - three' {
    run $cocomm -n2 "[1] 0x$D1 reset comm"
    assert_success
    assert_output "[1] $OK
000#82$D1
7$D1#00"
    run $cocomm -n2 "[2] 0x$D2 reset comm"
    assert_success
    assert_output "[2] $OK
000#82$D2
7$D2#00"
}

@test 'all preoperational' {
    assert $cocomm -n1 "0 preoperational"
}

@test 'SDO: disable all PDOs on D1' {
    run $cocomm -n0 "[11] 0x$D1 w 0x1400 1 u32 0x800002$D1" \
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
    run $cocomm -n16 "[21] 0x$D2 w 0x1400 1 u32 0x800002$D2" \
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

@test 'NMT: pre-operational D1' {
    run $cocomm -n2 "[1] 0x$D1 w 0x1017 0 i16 0x$HB"
    assert_success
    assert_output "[1] $OK
7$D1#7F
7$D1#7F"
}
@test 'NMT: operational D1' {
    run $cocomm -n3 "[1] 0x$D1 start"
    assert_success
    assert_output "[1] $OK
000#01$D1
7$D1#05
7$D1#05"
    assert $cocomm -n0 "0x$D1 w 0x1017 0 i16 0"
}

@test 'NMT: pre-operational D2' {
    run $cocomm -n4 "[1] 0x$D2 w 0x1017 0 i16 0x$HB"
    assert_success
    assert_output "[1] $OK
6$D2#2B171000"$HB"000000
5$B2#6017100000000000
7$D2#7F
7$D2#7F"
}
@test 'NMT: operational D2' {
    run $cocomm -n3 "[1] 0x$D2 start"
    assert $cocomm -n2 "0x$D2 w 0x1017 0 i16 0"
    assert_success
    assert_output "[1] $OK
000#01$D2
7$D2#05
7$D2#05"
}

@test 'finish: NMT all preoperational' {
    assert $cocomm -n1 "0 preoperational"
}
