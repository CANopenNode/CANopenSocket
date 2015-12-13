CANopenSocket
=============

CANopenSocket is a collection of CANopen tools running on Linux with socketCAN interface.
It is based on CANopenNode, which is an opensource [CANopen](http://can-cia.org/) Stack
([CiA301](http://can-cia.org/standardization/technical-documents)) and is included as a git submodule.

CANopenSocket may be used as a master or a slave device. However, CANopen itself is not a
typical master/slave protocol. It is more like producer/consumer protocol. It is also
possible to operate CANopen network without a master. Pre-configured process data (PDO)
are transmitted from producers. Each PDO may be consumed by multiple nodes.

CANopen master of CANopenNode functionality contains command line interface with SDO and NMT master
commands. With SDO master (or SDO client) it is possible to read or write any variable on any device
on the CANopen Network. NMT master can start, stop or reset nodes.

CANopenNode should run on any Linux machine. Examples below was tested on Debian based machines,
including **Ubuntu**, **Beaglebone Black** and **Raspberry PI**. It is possible to run tests described
below without real CAN interface, because Linux kernel already contains virtual CAN interface.

CANopenSocket consists of two applocations: **canopend**, which runs in background, and
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


Getting started with CANopen Socket
-----------------------------------

We will run two instances of *CANopend*. First will be basic node with ID=4,
second, with nodeID = 3, will have master functionality.


### Get the project

Clone the project from git repoitory and get submodules:
    $ git clone https://github.com/CANopenNode/CANopenSocket.git
    $ cd CANopenSocket
    $ git submodule init
    $ git submodule update

(If you want to work on submodule CANopenNode, you can apply git commands directly on it:)
    $ cd CANopenNode
    $ git checkout master
    $ git remote -v
    $ git remote set-url origin {url-of-your-git-repository}
    $ git remote add {yourName} {url-of-your-git-repository} # alternative
    $ git pull ({yourName} {yourbranch})
    $ # etc.


### First terminal: CAN dump

Prepare CAN virtual (or real) device:
    $ sudo modprobe vcan
    $ sudo ip link add dev vcan0 type vcan
    $ sudo ip link set up vcan0

Run candump from [can-utils](https://github.com/linux-can/can-utils):
    $ sudo apt-get install can-utils
    $ candump vcan0

It will show all CAN trafic on vcan0.


### Second terminal: canopend

Start second terminal, compile and start *canopend*.
    $ cd CANopenSocket/canopend
    $ make
    $ ./canopend --help
    $ ./canopend vcan0 -i 4 -s od4_storage -a od4_storage_auto

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
    $ ./canopend vcan0 -i 4 -s od4_storage -a od4_storage_auto

    vcan0  704   [1]  00
    vcan0  184   [2]  00 00                     # PDO message
    vcan0  704   [1]  05

Now there is operational state (0x05) and there shows one PDO on CAN
address 0x184. To learn more about PDOs, how to configure communication
and mapping parameters and how to use them see other sources of CANopen
documentation.

Start also second instance of *canopend* (master on nodeID=3) in the same
window (*canopend terminal*). Use default od_storage files and default
socket for command interface.
    $ # press CTRL-Z
    $ bg
    $ ./canopend vcan0 -i 3 -c ""


### Third terminal: canopencomm

Start third terminal, compile and start canopencomm.
    $ cd CANopenSocket/canopencomm
    $ make
    $ ./canopencomm --help

#### SDO master

Play with it and also observe CAN dump teminal. First Hertbeat at
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

With [CANopenNode](https://github.com/CANopenNode/CANopenNode) you can also design your
own device. There are many very useful and high quality specifications for different
[device profiles](http://www.can-cia.org/standardization/specifications/),
some of them are public and free to download.

Here we played with virtual CAN interface and result shows as pixels on
screen. If you connect real CAN interface to your computer, things may
become dangerous. Keep control on your machines!

