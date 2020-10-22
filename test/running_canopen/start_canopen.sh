#!/bin/bash

export can_device=can_test
export cleanup=tmp/stop_canopen.sh

# if first argument is not specified, run all four CANopen devices
if [ -z "$1" ] ; then
    export mask=1234
else
    export mask=$1
fi

# if "can_device" does not exist, create virtual CAN device
if ! grep -q $can_device /proc/net/can/rcvlist_all ; then
    echo "ip link: adding and setting up virtual CAN device named '$can_device'"
    sudo modprobe vcan
    sudo ip link add dev $can_device type vcan
    sudo ip link set up $can_device
fi

# prepare files for storage
mkdir tmp
echo "-" > tmp/od1_storage
echo "-" > tmp/od1_storage_auto
echo "-" > tmp/od4_storage
echo "-" > tmp/od4_storage_auto
echo "-" > tmp/od99_storage
echo "-" > tmp/od99_storage_auto
echo "-" > tmp/od7f_storage
echo "-" > tmp/od7f_storage_auto

echo "Starting several CANopen devices on \'$can_device\' in backgroud..."

# prepare stop script and run up to four CANopen devices in background
echo "#!/bin/bash" > $cleanup
chmod a+x $cleanup

if [[ "$mask" =~ "1" ]] ; then
    ./canopend $can_device -i 1 -s tmp/od1_storage -a tmp/od1_storage_auto -c "/tmp/CO_command_socket"&
    echo kill $! >> $cleanup
    sleep 0.1
fi
if [[ "$mask" =~ "2" ]] ; then
    ./canopend $can_device -i 4 -s tmp/od4_storage -a tmp/od4_storage_auto&
    echo kill $! >> $cleanup
    sleep 0.1
fi
if [[ "$mask" =~ "3" ]] ; then
    ./canopend $can_device -i 99 -s tmp/od99_storage -a tmp/od99_storage_auto&
    echo kill $! >> $cleanup
    sleep 0.1
fi
if [[ "$mask" =~ "4" ]] ; then
    ./canopend $can_device -i 127 -s tmp/od7f_storage -a tmp/od7f_storage_auto&
    echo kill $! >> $cleanup
    sleep 0.1
fi

echo "rm -r tmp" >> $cleanup

echo -e "\nrun '$cleanup' for cleanup.\n"
