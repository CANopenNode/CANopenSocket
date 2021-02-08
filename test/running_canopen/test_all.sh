#!/bin/bash

if [[ $1 = "" ]] ; then
    echo "No CAN device specified. Assuming two CANopen devices already running:"
    echo " - 'canopend' with nodeId=1 and gateway interface: 'local-/tmp/CO_command_socket'"
    echo " - 'basicDevice' with nodeId=4"
    echo "If that is not the case, read test.md or try: ./test_all.sh vcan0"
else
    echo "Starting two CANopen devices in background with 'start_canopen.sh':"
    ./start_canopen.sh $1
fi


START=$(date +%s.%N)

echo -e "\nRunning 'basic_NMT_SDO_heartbeat.bats':"
./basic_NMT_SDO_heartbeat.bats
echo -e "\nRunning 'heartbeat_consumer.bats':"
./heartbeat_consumer.bats
echo -e "\nRunning 'SDO_transfer.bats':"
./SDO_transfer.bats

END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo -e "Test duration: $DIFF sec\n"


if [[ $1 != "" ]] ; then
    echo "Cleanup two CANopen devices:"
    tmp/stop_canopen.sh
fi

echo -e "\nPress enter to close..."
read
