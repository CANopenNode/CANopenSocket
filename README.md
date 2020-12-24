CANopenSocket
=============

CANopenSocket is a collection of CANopen tools and examples.

It is based on [CANopenNode](https://github.com/CANopenNode/CANopenNode), which is free and open source CANopen Stack and is included as a git submodule.

CANopen is the internationally standardized (EN 50325-4) ([CiA301](http://can-cia.org/standardization/technical-documents)) CAN-based higher-layer protocol for embedded control system. For more information on CANopen see http://www.can-cia.org/

**THIS RELEASE OF CANopenSocket CONTAINS NEW, UNFINISHED EDITION OF CANopenNode V4. PDO AND SOME OTHER PARTS ARE UNFINISHED.**

Getting or updating the project
-------------------------------
Clone the project from git repository and get submodules:

    git clone https://github.com/CANopenNode/CANopenSocket.git
    cd CANopenSocket
    git submodule init
    git submodule update

Update the project:

    cd CANopenSocket
    # make sure, project is clean (git status); changes should be committed (git gui)
    git pull # or: git fetch -> gitk --all (inspect the changes) -> git merge
    git submodule update

Get/update other tools (EDSeditor):

    cd tools
    ./get_tools.sh


Getting started
---------------
First information is in CANopenNode/README.

Basic getting started guide is available in `CANopenNode/doc/gettingStarted.md`. If everything works there as expected, with virtual or real CAN, then CANopen network is established. With at least basic operation of: CAN reception, transmission, NMT, Heartbeat, Service Communication Objects (SDO).

Next is Basic CANopen Device example program with source code, custom Object Dictionary access functions and advanced CANopen gateway communication. See [basicDevice/README.md](examples/basicDevice/README.md).


CANopenSocket contents
----------------------
 - **CANopenNode** - free and open source CANopen Stack (git submodule)
 - **docs** - CANopenNode documentation, generated by doxygen, available also online at https://canopennode.github.io/CANopenSocket/
 - **cocomm** - Linux commmand line tool, which communicates with CANopen gateway device via local or TCP socket. Similar as `nc` tool available in Linux. `make` the executable and type `./cocomm --help` or see [basicDevice/README.md](examples/basicDevice/README.md) for example usage.
 - **canopencgi** - CGI interface for Apache web server, experimental.
 - **tools**
   - **EDSEditor** - Object Dictionary Editor from https://github.com/robincornelius/libedssharp. Updated with `get_tools.sh`.
   - **get_tools.sh** - script for getting/updating the tools.
   - **update_docs.sh** - script runs doxygen on CANopenNode, copies documentation to `docs/` directory and creates softlink index.html.
   - **vcan0_candump.sh** - script creates virtual CAN device `vcan0` and runs `candump` on it.
 - **examples**
   - **basicDevice** - Example CANopenNode device with or without gateway interface. Includes example for different testing variables: 64-bit integers, real numbers, strings, domain, object oriented principle with C, etc. See [basicDevice/README.md](examples/basicDevice/README.md).
 - **test** - Different CANopen testing tools
   - **libs/bats-*** [Bash Automated Testing System](https://github.com/bats-core/bats-core), included as git submodules.
   - **running_canopen** - tests on running CANopen network, based on bats. On virtual or real CAN network. See [test.md](test/test.md)
 - **docker** - Dockerfile with readme.


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
