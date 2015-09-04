CANopenSocket
=============

CANopenSocket is a collection of CANopen tools running on Linux with socketCAN interface.
It is based on CANopenNode, which is an opensource [CANopen](http://can-cia.org/) Stack ([CiA301](http://can-cia.org/standardization/technical-documents)) and is included as a git submodule.

canopend
--------

canopend is an implementation of CANopen device with master functionality.


**First words on CAN bus (update 2):**

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

Now let's try to do some raw SDO access. We will access Object Dictionary(OD) variable "Producer heartbeat time". Inside OD it is on index 0x1017, subindex 0x00, as specified by standard. Curent default value is 1000. This means, Heartbeat is sent every 1000 milliseconds. Let's simulate the master and read the value:

    $ cansend vcan0 603#40.1710.00.00000000

candump shows response:

    vcan0  583   [8]  4B 17 10 00 E8 03 00 00

We sent command on CAN address 0x600+nodeId and received response from CAN address 0x580+nodeId. First data byte from command or response is SDO command specifier. Response includes length information (two bytes). For more info see file CO_SDO.h. "1710" actually means 0x1017. Please note, CANopen itself is little endian. Fourth byte is subindex. "E8 03" from response means 0x03E8, which is equivalent to 1000. 
Now change Heartbeat producer, so it will send heartbeats every 5 seconds (5000 == 0x1388):

    $ cansend vcan0 603#2B.1710.00.8813.0000

candump shows, that Heartbeat interval is now longer.
This way it is possible to access any variable not longer than 4 bytes on any CANopen device. However, this is only basic, raw access.


*At this point canopend may be fully functional CANopen device.* (Still no master functionality.)


To be continued ...

