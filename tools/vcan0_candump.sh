#!/bin/bash

export can_device=vcan0

# if "vcan0" does not exist, create it
if ! grep -q $can_device /proc/net/can/rcvlist_all ; then
    echo "ip link: adding and setting up virtual CAN device named '$can_device'"
    sudo modprobe vcan
    sudo ip link add dev $can_device type vcan
    sudo ip link set up $can_device
fi

candump -td $can_device
