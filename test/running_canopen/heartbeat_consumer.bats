#!/usr/bin/env ../libs/bats-core/bin/bats
load '../libs/bats-support/load.bash'
load '../libs/bats-assert/load.bash'

setup_file() {
    . ./config_bats.sh
}

@test 'Start heartbeat producer on D1 only' {
    assert $cocomm -n8 "[1] 0x$D2 w 0x1017 0 i16 0" \
                       "[2] 0x$D2 w 0x1016 2 u32 0" \
                       "[3] 0 reset communication" \
                       "[4] 0x$D1 w 0x1017 0 i16 0x$HB"
    run $cocomm -n2 "0 preoperational"
    assert_success
    assert_output "[1] $OK
000#8000
7$D1#7F"
}

@test 'Start heartbeat consumer on D2' {
    run $cocomm -n4 "[1] 0x$D2 w 0x1016 2 u32 0x00"$D1"00$HB_timeout"
    assert_success
    assert_output "[1] $OK
6$D2#23161002"$HB_timeout"000100
5$B2#6016100200000000
7$D1#7F
7$D1#7F"
}

@test 'Heartbeat producer D1 missing' {
    run $cocomm -n1 "[1] 0x$D1 w 0x1017 0 i16 0"
    assert_success
    assert_output "[1] $OK
0$B2#3081101B01000000"
}

@test 'Heartbeat producer D1 established' {
    run $cocomm -n3 "[1] 0x$D1 w 0x1017 0 i16 0x$HB"
    assert_success
    assert_output "[1] $OK
7$D1#7F
0$B2#0000001B00000000
7$D1#7F"
}

@test 'Bootup message D1' {
    run $cocomm -n6 "0x$D1 reset communication"
    assert_success
    assert_output "[1] $OK
000#8201
7$D1#00
0$B2#3081101C01000000
7$D1#05
0$B2#0000001C00000000
7$D1#05"
}

@test 'Second HB consumer configuration' {
    assert $cocomm -n2 "[1] 0x$D2 w 0x1016 3 u32 0x005500$HB_timeout"
}

@test 'Illegal HB consumer configuration (duplicate nodeId)' {
    refute $cocomm -n2 "[1] 0x$D2 w 0x1016 3 u32 0x00"$D1"00$HB_timeout"
}

@test 'Cleanup' {
    assert $cocomm -n7 "[1] 0x$D1 w 0x1017 0 i16 0" \
                       "[2] 0x$D2 w 0x1016 2 u32 0" \
                       "[2] 0x$D2 w 0x1016 3 u32 0" \
                       "[3] 0 reset communication"
}
