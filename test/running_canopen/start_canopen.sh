#!/bin/bash

if [[ $1 = "" ]] ; then
    echo "Usage: $0 <CAN device>"
    exit
else
    export can_device=$1
fi

if [[ $co_device_1 = "" ]] ; then
    export co_device_1="../../CANopenNode/canopend"
fi
if [[ $co_device_4 = "" ]] ; then
    export co_device_4="../../examples/basicDevice/basicDevice"
fi

export cleanup=tmp/stop_canopen.sh

# if "can_device" does not exist, create virtual CAN device
if ! grep -q $can_device /proc/net/can/rcvlist_all ; then
    echo "ip link: adding and setting up virtual CAN device named '$can_device'"
    sudo modprobe vcan
    sudo ip link add dev $can_device type vcan
    sudo ip link set up $can_device
fi

# prepare files for storage
mkdir tmp

# prepare stop script and run CANopen devices in background
echo "#!/bin/bash" > $cleanup
chmod a+x $cleanup

echo "Running in background: '$co_device_1 $can_device -i 1 -s \"tmp/dev1_\" -c \"local-/tmp/CO_command_socket\"'"
echo "-" > tmp/dev1_lss.persist
echo "-" > tmp/dev1_od_comm.persist
$co_device_1 $can_device -i 1 -s "tmp/dev1_" -c "local-/tmp/CO_command_socket"&
echo kill $! >> $cleanup
sleep 0.1
echo

echo "Running in background: '$co_device_4 $can_device -i 4 -s \"tmp/dev4_\"'"
echo "-" > tmp/dev4_lss.persist
echo "-" > tmp/dev4_od_comm.persist
echo "-" > tmp/dev4_od_test_auto.persist
echo "-" > tmp/dev4_od_test.persist
$co_device_4 $can_device -i 4 -s "tmp/dev4_"&
echo kill $! >> $cleanup
sleep 0.1
echo

echo "rm -r tmp" >> $cleanup

echo "For cleanup run: $cleanup"
echo
