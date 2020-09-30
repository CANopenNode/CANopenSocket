CANopenSocket
=============

CANopenSocket is a collection of CANopen tools running on Linux with socketCAN interface.

It is based on [CANopenNode](https://github.com/CANopenNode/CANopenNode),
which is free and open source CANopen Stack and is included as a git submodule.

CANopen is the internationally standardized (EN 50325-4)
([CiA301](http://can-cia.org/standardization/technical-documents))
CAN-based higher-layer protocol for embedded control system. For more
information on CANopen see http://www.can-cia.org/

CANopenSocket may be used as a master or a slave device. However, CANopen itself is not a
typical master/slave protocol. It is more like producer/consumer protocol. It is also
possible to operate CANopen network without a master. Pre-configured process data (PDO)
are transmitted from producers. Each PDO may be consumed by multiple nodes.

Master functionality of CANopenNode contains command line interface with SDO and NMT master
commands. With SDO master (or SDO client) it is possible to read or write any variable on any device
on the CANopen Network. NMT master can start, stop or reset nodes.

CANopenNode should run on any Linux machine. Examples below was tested on Debian based machines,
including **Ubuntu**, **Beaglebone Black** and **Raspberry PI**. It is possible to run tests described
below without real CAN interface, because Linux kernel already contains virtual CAN interface.

CANopenSocket consists of two applications: **canopend**, which runs in background, and
**canopencomm**, command interface for SDO and NMT master.


### canopend
**canopend** is an implementation of CANopen device with master functionality. It runs within three
threads. Realtime thread processes CANopen SYNC and PDO objects. Mainline thread processes other
non time critical objects. Both are nonblocking. Command interface thread is blocking. It accepts
commands from socket connection from external application and executes master SDO and NMT tasks.


### canopencomm
**canopencomm** is the other end of the Command interface. It accepts text commands form arguments
or from standard input or from file. It sends commands to *canopend* via socket, line after line.
Received result is printed to standard output. It is implementation of the CiA 309 standard.


License
-------
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Getting started with CANopen Socket
-----------------------------------

canopend with command interface and "Getting started" is now availabe in base CANopenNode repo. See https://github.com/CANopenNode/CANopenNode/blob/master/doc/gettingStarted.md

**Example below is outdated**

-------------------------

We will run two instances of *CANopend*. First will be basic node with ID=4,
second, with nodeID = 3, will have master functionality.


### Get the project

Clone the project from git repository and get submodules:

    $ git clone https://github.com/CANopenNode/CANopenSocket.git
    $ cd CANopenSocket
    $ git submodule init
    $ git submodule update

If you already have the project and just want to update, use:

    cd CANopenSocket
    git pull # or: git fetch; inspect the changes (gitk); git merge
    git submodule update


(If you want to work on submodule CANopenNode, you can `cd CANopenNode`,
and apply git commands directly on it. Initially is in head detached state,
so you have to `git checkout master` first. Then you can control submodule
separately, for example `git remote add {yourName} {url-of-your-git-repository}`,
and `git pull {yourName} {yourbranch}`)


### First terminal: CAN dump

Prepare CAN virtual (or real) device:

    $ sudo modprobe vcan
    $ sudo ip link add dev vcan0 type vcan
    $ sudo ip link set up vcan0

Run candump from [can-utils](https://github.com/linux-can/can-utils):

    $ sudo apt-get install can-utils
    $ candump vcan0

It will show all CAN traffic on vcan0.


### Second terminal: canopend

Start second terminal, compile and start *canopend*.

    $ cd CANopenSocket/canopend
    $ make
    $ app/canopend --help
    $ app/canopend vcan0 -i 4 -s od4_storage -a od4_storage_auto

You should now see CAN messages on CAN dump terminal. Wait few seconds and
press CTRL-C.

    vcan0  704   [1]  00                        # Bootup message.
    vcan0  084   [8]  00 50 01 2F F3 FF FF FF   # Emergency message.
    vcan0  704   [1]  7F                        # Heartbeat messages
    vcan0  704   [1]  7F                        # one per second.

Heartbeat messages shows pre-operational state (0x7F). If you follow byte 4 of the
Emergency message into [CANopenNode/stack/CO_Emergency.h],
CO_EM_errorStatusBits, you will see under 0x2F "CO_EM_NON_VOLATILE_MEMORY",
which is generic, critical error with access to non volatile device memory.
This byte is CANopenNode specific. You can observe also first two bytes,
which shows standard error code (0x5000 - Device Hardware) or third byte,
which shows error register. If error register is different than zero, then
node is not able to enter operational and PDOs can not be exchanged with it.

You can follow the reason of the problem inside the source code. However,
there are missing non-default storage files. Add them and run it again.

    $ echo - > od4_storage
    $ echo - > od4_storage_auto
    $ app/canopend vcan0 -i 4 -s od4_storage -a od4_storage_auto

    vcan0  704   [1]  00
    vcan0  184   [2]  00 00                     # PDO message
    vcan0  704   [1]  05

Now there is operational state (0x05) and there shows one PDO on CAN
address 0x184. To learn more about PDOs, how to configure communication
and mapping parameters and how to use them see other sources of CANopen
documentation (For example article of PDO re-mapping procedure in [CAN
newsletter magazine, June 2016](http://can-newsletter.org/engineering/engineering-miscellaneous/160601_can-newsletter-magazine-june-2016) ).

Start also second instance of *canopend* (master on nodeID=3) in the same
window (*canopend terminal*). Use default od_storage files and default
socket for command interface.

    $ # press CTRL-Z
    $ bg
    $ app/canopend vcan0 -i 3 -c ""


### Third terminal: canopencomm

Start third terminal, compile and start canopencomm.

    $ cd CANopenSocket/canopencomm
    $ make
    $ ./canopencomm --help

#### SDO master

Play with it and also observe CAN dump terminal. First Heartbeat at
index 0x1017, subindex 0, 16-bit integer, on nodeID 4.

    $ ./canopencomm [1] 4 read 0x1017 0 i16
    $ ./canopencomm [1] 4 write 0x1017 0 i16 5000

In CAN dump you can see some SDO communication. You will notice, that
Heartbeats from node 4 are coming in 5 second interval now. You can do
the same also for node 3. Now store Object dictionary, so it will preserve
variables on next start of the program.

    $ ./canopencomm 4 w 0x1010 1 u32 0x65766173

You can read more about Object dictionary variables for this
CANopenNode in [canopend/CANopenSocket.html].


#### NMT master
If node is operational (started), it can exchange all objects, including
PDO, SDO, etc. In pre-operational, PDOs are disabled, SDOs works. In stopped
only NMT messages are accepted.

    $ ./canopencomm 4 preop
    $ ./canopencomm 4 start
    $ ./canopencomm 4 stop
    $ ./canopencomm 4 r 0x1017 0 i16 		# time out
    $ ./canopencomm 4 reset communication
    $ ./canopencomm 4 reset node
    $ ./canopencomm 3 reset node

In *canopend terminal* you see, that both devices finished. Further commands
are not possible. If you set so, last command can also reset computer.

#### Combining NMT commands into a single file

Create a `commands.txt` file, and for its content enter your commands.
Example:

    [1] 3 start
    [2] 4 start

Make canopencomm use that file:

    $ ./canopencomm -f commands.txt
    [1] OK
    [2] OK

### CANOpen network client

By default **canopencomm** and **canopend** opens a local unix socket to interact.
That's fine for local usage, but note that TCP sockets are also supported.
Hence we can interact with our devices from a remote client.

To interact with **canopend** over TCP, specify its TCP port using the -t argument.
On our embedded linux device:

    $ ./canopend can0 -i 1 -c "" -t 6000

On our linux pc we need to specify the target device using -t, and its port using -p:

    $ ./canopencomm -t 192.168.0.2 -p 6000

### Next steps
Now you can learn more skills on CANopen from some other sources:
books, data sheet of some CANopen device, standard CiA 301(it's free), etc.
Then you can enter the big world of [CANopen devices](http://can-newsletter.org/hardware).


Accessing real CANopen devices is the same as described above for virtual CAN interface.
Some tested USB to CAN interfaces, which are natively integrated into Linux are:

 - Simple serial [USBtin](http://www.fischl.de/usbtin/) - Start with: `sudo slcand -f -o -c -s8 /dev/ttyACM0 can0; sudo ip link set up can0`
 - [EMS CPC-USB](http://www.ems-wuensche.com/product/datasheet/html/can-usb-adapter-converter-interface-cpcusb.html) - Start with: `sudo ip link set up can0 type can bitrate 250000`
 - [PCAN-USB FD](http://www.peak-system.com/PCAN-USB-FD.365.0.html?&L=1) - Needs newer Linux kernel, supports CAN flexible data rate.
 - You can get the idea of other supported CAN interfaces in [Linux kernel source](https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/drivers/net/can).
 - Beaglebone or Paspberry PI or similar has CAN capes available. On RPI worked
   also the above USB interfaces, but it was necessary to compile the kernel.


With [CANopenNode](https://github.com/CANopenNode/CANopenNode) you can also design your
own device. There are many very useful and high quality specifications for different
[device profiles](http://www.can-cia.org/standardization/specifications/),
some of them are public and free to download.


Here we played with virtual CAN interface and result shows as pixels on
screen. If you connect real CAN interface to your computer, things may
become dangerous. Keep control and safety on your machines!


### LSS extension
LSS (Layer settings service) is a extension to CANopen described in CiA DSP 305. The
interface is described in CiA DS 309 2.1.0 (ASCII mapping).
LSS allows the user to change node ID and bitrate, as well as setting the node ID on an
unconfigured node.

LSS uses the the OD Identity register as an unique value to select a node. Therefore
the LSS address always consists of four 32 bit values. This also means that LSS relies
on this register to actually be unique.

To use LSS, a compatible node is needed. Note that canopend only includes LSS master
functionality.

The following example show some typical use cases for LSS:

 - Changing the node ID for a known slave, store the new node ID to eeprom, apply new node ID.
   The node currently has the node ID 22.

        $ ./canopencomm lss_switch_sel 0x00000428 0x00000431 0x00000002 0x5C17EEBC
        $ ./canopencomm lss_set_node 4
        $ ./canopencomm lss_store
        $ ./canopencomm lss_switch_glob 0
        $ ./canopencomm 22 reset communication

   Note that the node ID change is not done until reset communication/node

 - Changing the node ID for a known slave, store the new node ID to eeprom, apply new node ID.
   The node currently has an invalid node ID.

        $ ./canopencomm lss_switch_sel 0x00000428 0x00000431 0x00000002 0x5C17EEBC
        $ ./canopencomm lss_set_node 4
        $ ./canopencomm lss_store
        $ ./canopencomm lss_switch_glob 0

   Note that the node ID is automatically applied.

 - Search for a node via LSS fastscan, store the new node ID to eeprom, apply new node ID

        $ ./canopencomm [1] _lss_fastscan

        [1] 0x00000428 0x00000432 0x00000002 0x6C81413C

        $ ./canopencomm lss_set_node 4
        $ ./canopencomm lss_store
        $ ./canopencomm lss_switch_glob 0

    To increase scanning speed, you can use

        $ ./canopencomm [1] _lss_fastscan 25

    where 25 is the scan step delay in ms. Be aware that the scan will become unreliable when
    the delay is set to low.

 - Auto enumerate all nodes via LSS fastscan. Enumeration automatically begins at node ID 2
   and node ID is automatically stored to eeprom. Like with _lss_fastscan, an optional
   parameter can be used to change default delay time.

        $ ./canopencomm lss_allnodes

        [1] OK, found 3 nodes starting at node ID 2.

 - To get further control over the fastscan process, the lss_allnodes command supports
   an extended parameter set. If you want to use this set, all parameters are mandatory.
   Auto enumerate all nodes via LSS fastscan. Set delay time to 25ms, set enumeration start
   to node ID 7, do not store LSS address in eeprom, enumerate only devices with vendor ID
   "0x428", ignore product code and software revision, scan for serial number

        $ ./canopencomm lss_allnodes 25 7 0 2 0x428 1 0 1 0 0 0

        [1] OK, found 2 nodes starting at node ID 7.

   The parameters are as following:
   - 25     scan step delay time in ms
   - 7      enumeration start
   - 0      store node IDs to eeprom; 0 = no, 1 = yes
   - 2      vendor ID scan selector; 0 = fastscan, 2 = match value in next parameter
   - 0x428  vendor ID to match
   - 1      product code scan selector; 0 = fastscan, 1 = ignore, 2 = match value in next parameter
   - 0      product code to match (ignored in this example)
   - 1      software version scan selector; 0 = fastscan, 1 = ignore, 2 = match value in next parameter
   - 0      software version to match (ignored in this example)
   - 0      serial number scan selector; 0 = fastscan, 1 = ignore, 2 = match value in next parameter
   - 0      serial number to match (not used in this example)

 Note that only unconfigured nodes (those without a valid node ID) will take part in
 fastscan!
