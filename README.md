CANopenSocket
=============

CANopenSocket is a collection of CANopen tools running on Linux with socketCAN interface.
It is based on CANopenNode, which is an opensource [CANopen](http://can-cia.org/) Stack ([CiA301](http://can-cia.org/standardization/technical-documents)) and is included as a git submodule.

canopend
--------

canopend is an implementation of CANopen device with master functionality.


**First words on CAN bus (updated):**

To clone the project with submodules use:

    $ git clone --recursive https://github.com/CANopenNode/CANopenSocket.git

Compile:

    $ cd CANopenSocket/canopend
    $ make

Now go to second terminal and prepare CAN (virtual) device:

    $ sudo modprobe vcan
    $ sudo ip link add dev vcan0 type vcan
    $ sudo ip link set up vcan0

From the same terminal run candump from [can-utils](https://github.com/linux-can/can-utils):

    $ candump vcan0

Now go to the first terminal and run canopend on vcan0, nodeId=3, no realtime scheduler:

    $ ./canopend vcan0 -i3 -p-1

Program will print some debug text, then it will print some nonsense line each second.

On second terminal will show something like this:

    vcan0  703   [1]  00
    vcan0  183   [2]  00 00
    vcan0  083   [8]  00 00 00 22 01 00 00 00
    vcan0  703   [1]  05
    vcan0  703   [1]  05
    vcan0  703   [1]  05
    ....

That is bootup message from nodeId=3, one PDO, informative emergency message (22=CO_EM_MICROCONTROLLER_RESET) and heartbeat messages in 1s interval informing that node is operational.

Now open the third terminal and try to send some manual NMT commands (CAN address 000), to nodeId=03. First send CO_NMT_ENTER_PRE_OPERATIONAL=0x80, then send CO_NMT_ENTER_OPERATIONAL=0x01:

    $ cansend vcan0 000#80.03
    $ cansend vcan0 000#01.03

candump in second terminal will show:

    ...
    vcan0  703   [1]  05
    vcan0  703   [1]  05
    vcan0  000   [2]  80 03
    vcan0  703   [1]  7F
    ...
    vcan0  703   [1]  7F
    vcan0  703   [1]  7F
    vcan0  000   [2]  01 03
    vcan0  183   [2]  00 00
    vcan0  703   [1]  05
    vcan0  703   [1]  05
    ...

canopend accepted NMT commands as seen in responded heartbeat messages (CO_NMT_OPERATIONAL=0x05, CO_NMT_PRE_OPERATIONA=0x7F). Event triggered PDO was resent after entered operational.

No further tests were made.

*At this point canopend may be fully functional CANopen device.* (No master functionality yet.)


To be continued ...

