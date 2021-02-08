TEST REPORTS
============

Complete tests in running CANopen
---------------------------------
`running_canopen/test_all.sh`, two CAN devices, 1000 kbps:
```
No CAN device specified. Assuming two CANopen devices already running:
 - 'canopend' with nodeId=1 and gateway interface: 'local-/tmp/CO_command_socket'
 - 'basicDevice' with nodeId=4
If that is not the case, read test.md or try: ./test_all.sh vcan0

Running 'basic_NMT_SDO_heartbeat.bats':
 ✓ NMT: all preoperational
 ✓ SDO: read heartbeat D1
 ✓ SDO: read heartbeat D2
 ✓ SDO: disable heartbeats
 ✓ candump: no communication, 2sec
 ✓ Heartbeats: all present
 ✓ NMT: reset communication - one
 ✓ NMT: reset communication - two
 ✓ NMT: reset communication - three
 ✓ all preoperational
 ✓ SDO: disable all PDOs on D1
 ✓ SDO: disable all PDOs on D2
 ✓ NMT: pre-operational D1
 ✓ NMT: operational D1
 ✓ NMT: pre-operational D2
 ✓ NMT: operational D2
 ✓ finish: NMT all preoperational

17 tests, 0 failures


Running 'heartbeat_consumer.bats':
 ✓ Start heartbeat producer on D1 only
 ✓ Start heartbeat consumer on D2
 ✓ Heartbeat producer D1 missing
 ✓ Heartbeat producer D1 established
 ✓ Bootup message D1
 ✓ Second HB consumer configuration
 ✓ Illegal HB consumer configuration (duplicate nodeId)
 ✓ Cleanup

8 tests, 0 failures


Running 'SDO_transfer.bats':
 ✓ Segmented transfer - set filename for upload
 ✓ Block transfer, create 100000 bytes long random binaryFile
 ✓ Block download binaryFile
 ✓ Block upload to binaryFileReceived - 2x
 ✓ Block transfer, files equal
 ✓ Segmented transfer, create 1000 bytes long random binaryFileSeg
 ✓ Segmented download binaryFileSeg
 ✓ Segmented upload to binaryFileSegReceived
 ✓ Segmented transfer, files equal
   100000 bytes block down/upload: 1845/2310 milliseconds.
   1000 bytes segmented down/upload: 256/280 milliseconds.

9 tests, 0 failures

Test duration: 13.637902731 sec
```


SDO transfer test reports for different target systems
------------------------------------------------------

Running tests with `SDO_transfer.bats`, see test.md for preparation of devices.

### Devices tested 2021-05-02
- systems:
  - Ubuntu 20.10, Linux 5.8.0-41-generic on desktop PC
  - Raspberry Pi OS 10 (buster) on RPI
- canopend, testing SDO client and gateway:
  - versions CANopenNode:
    - "v4.0-25-g8855820"
    - "v2.0-6-gacd719e"
- basicDevice, testing SDO server
  - version CANopenSocket "v4.0-2-geb7d5f3-dirty"
  - version CANopenNode "v4.0-25-g8855820" (v2.0 was not tested)
- CAN interfaces:
  - CPC-USB, EMS Dr. Thomas Wuensche
  - PCAN-USB: PEAK-System PCAN-USB FD v1 fw v3.2.0 (1 channel, FD not used)
  - USBtin: USBtin EB v2.0 (slcand)
  - Raspberry Pi 2 (and 4) Model B
    - [CAN shield with MCP2515](https://www.sg-electronic-systems.com/can-bus-dual-iso-v2-1-shield-for-raspberry/)
    - CPC-USB (PCAN-USB didn't work)

### Virtual CAN device
Test result:
```
 ✓ Segmented transfer - set filename for upload
 ✓ Block transfer, create 100000 bytes long random binaryFile
 ✓ Block download binaryFile
 ✓ Block upload to binaryFileReceived - 2x
 ✓ Block transfer, files equal
 ✓ Segmented transfer, create 1000 bytes long random binaryFileSeg
 ✓ Segmented download binaryFileSeg
 ✓ Segmented upload to binaryFileSegReceived
 ✓ Segmented transfer, files equal
   100000 bytes block down/upload: 70/145 milliseconds.
   1000 bytes segmented down/upload: 10/24 milliseconds.
```

### CAN bus 1000 Kbps
Test result:
```
 ✓ Segmented transfer - set filename for upload
 ✓ Block transfer, create 100000 bytes long random binaryFile
 ✓ Block download binaryFile
 ✓ Block upload to binaryFileReceived - 2x
 ✓ Block transfer, files equal
 ✓ Segmented transfer, create 1000 bytes long random binaryFileSeg
 ✓ Segmented download binaryFileSeg
 ✓ Segmented upload to binaryFileSegReceived
 ✓ Segmented transfer, files equal
   100000 bytes block down/upload: 2316/1840 milliseconds.
   1000 bytes segmented down/upload: 254/270 milliseconds.
```
#### Notes USBtin
USBtin interface doesn't work when sending sequence of data. In block transfer it sends first CAN message in sub-block correctly, then there is a mess on the bus. However data was transferred correctly in all test cases, because of the robustness of the block transfer. But communication time is up to 300 times longer. Reception of the sequence of data as well as segmented transfer always works correctly with USBtin.

#### Notes Raspberry PI with CAN shield MCP2515
Raspberry PI shows had problems with CAN overflow with received sequence of data. It couldn't process fast enough and there was socket overflow. RPI sends emergency message and sets error register. Otherwise data transfer was always correct and time to transfer was not significantly longer. With 250 Kbps there were still overflows, but less. Problem was the same, when RPI was basicDevice (multithreaded program) or canopend (single threaded program). Problem was also the same, when using RPI2 or RPI4. But everything works perfectly when using CPC-USB instead of CAN shield with RPI2 or RPI4.
```
basicDevice: (CO_CANerror_rxMsgError) Socket error msg ID: 0x20000004, Data[0..7]: 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 (can0)
basicDevice: CAN Interface "can0" Rx buffer overflow. Message dropped
canopend: CANopen Emergency message from node 0x04: errorCode=0x0013, errorRegister=0x10, errorBit=0x13, infoCode=0x00000000
```

#### Notes other
No problems were detected on other CAN interfaces.


### CAN bus 10 Kbps
Enlarge SDO timeout and reduce data size:
```
cocomm "set sdo_timeout 2000"
export binaryFileSize=1000
```

Test result:
```
 ✓ Segmented transfer - set filename for upload
 ✓ Block transfer, create 1000 bytes long random binaryFile
 ✓ Block download binaryFile
 ✓ Block upload to binaryFileReceived - 2x
 ✓ Block transfer, files equal
 ✓ Segmented transfer, create 1000 bytes long random binaryFileSeg
 ✓ Segmented download binaryFileSeg
 ✓ Segmented upload to binaryFileSegReceived
 ✓ Segmented transfer, files equal
   1000 bytes block down/upload: 1741/1731 milliseconds.
   1000 bytes segmented down/upload: 3611/3621 milliseconds.
```

#### Notes
USBtin shows the same problems as with 1000 Kbps. All others worked perfectly.
