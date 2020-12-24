Test
====

Tests in Running CANopen network
--------------------------------

Tests on Running CANopen network are implemented with one CANopenNode Linux device with gateway interface and three additional CANopenNode devices. Tests are run in Linux command line with [Bash Automated Testing System](https://github.com/bats-core/bats-core), which is included as three git submodules.

To run tests automatically, simply type (after compiling `cocomm` and `canopend`):

    cd test/running_canopen$
    ./test_canopend.sh

This will run four instances of canopend, run basic_NMT_SDO_heartbeat tests and do a cleanup.

### Manual preparation of CANopen devices

1. Create Linux CAN device named `can_test`. Virtual CAN example:
    ```
    sudo modprobe vcan
    sudo ip link add dev can_test type vcan
    sudo ip link set up can_test
    ```

2. Start CANopenNode commander somewhere with NodeID: `1` and local socket on `/tmp/CO_command_socket_test`:
    ```
    ./canopend can_test -i 1 -c "local-/tmp/CO_command_socket_test"
    ```

3. Start three CANopen devices with NodeIDs: `4`, `99` and `0x7F`:
    ```
    ./canopend can_test -i 4
    ./canopend can_test -i 99
    ./canopend can_test -i 127
    ```
### Manually running the tests

    ./basic_NMT_SDO_heartbeat.bats

Open the `basic_NMT_SDO_heartbeat.bats` script file and examine the contents. Script is quite straightforward. Command line program `cocomm` is used for running each test. For `cocomm` example usage see [basicDevice/README.md](../examples/basicDevice/README.md). Program uses here one additional functionality - it prints dump of actual CAN messages, which occur with each `cocomm` command. So BATS can verify the complete result.
