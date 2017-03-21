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
CANopenSocket is free and open source software: you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.


Getting started with CANopen Socket
-----------------------------------

We will run two instances of *CANopend*. First will be basic node with ID=4,
second, with nodeID = 3, will have master functionality.


### Get the project

Clone the project from git repository and get submodules:

    $ git clone https://github.com/CANopenNode/CANopenSocket.git
    $ cd CANopenSocket
    $ git submodule init
    $ git submodule update

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

