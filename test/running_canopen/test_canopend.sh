#!/bin/bash

echo "Starting start_canopen$1.sh:"
./start_canopen$1.sh
START=$(date +%s.%N)

echo "Running basic_NMT_SDO_heartbeat.bats:"
./basic_NMT_SDO_heartbeat.bats
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo -e "Test duration: $DIFF sec\n"

tmp/stop_canopen.sh

echo "press enter to continue..."
read
