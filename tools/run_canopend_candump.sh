#!/bin/bash

if [[ $1 = "" ]] ; then
    if [[ $can_device = "" ]] ; then
        can_device=vcan0
    fi
else
    can_device=$1
fi
echo "Using CAN device: '$can_device'"

# if "can_device" does not exist, create it
if ! grep -q $can_device /proc/net/can/rcvlist_all ; then
    echo "ip link: adding and setting up virtual CAN device named '$can_device'"
    sudo modprobe vcan
    sudo ip link add dev $can_device type vcan
    sudo ip link set up $can_device
fi

echo "Running in background: 'candump -td $can_device"
candump -td $can_device&


if [[ $2 = "" ]] ; then
    if [[ $co_device = "" ]] ; then
        co_device="../CANopenNode/canopend"
    fi
else
    co_device=$2
fi

mkdir tmp
echo "Running: '$co_device $can_device -i 1 -s \"tmp/dev1_\" -c \"local-/tmp/CO_command_socket\"'"
echo "-" > tmp/dev1_lss.persist
echo "-" > tmp/dev1_od_comm.persist
$co_device $can_device -i 1 -s "tmp/dev1_" -c "local-/tmp/CO_command_socket"

rm -r tmp

echo -e "\nFinished, press enter to close..."
read
