CANopen documentation
=====================
**Basic device**

Basic CANopen device with example usage.

|              |                                |
| ------------ | ------------------------------ |
| Project File | basicDevice.xdd                |
| File Version | 1                              |
| Created      | 28. 11. 2020 12:37:54          |
| Created By   | Janez Paternoster              |
| Modified     | 1. 02. 2021 15:30:19           |
| Modified By  | Janez Paternoster              |

This file was automatically generated with [libedssharp](https://github.com/robincornelius/libedssharp) Object Dictionary Editor v0.8-116-g3349c4d

* [Device Information](#device-information)
* [PDO Mapping](#pdo-mapping)
* [Communication Specific Parameters](#communication-specific-parameters)
* [Manufacturer Specific Parameters](#manufacturer-specific-parameters)
* [Device Profile Specific Parameters](#device-profile-specific-parameters)


Device Information {#device-information}
----------------------------------------
|              |                                |
| ------------ | ------------------------------ |
| Vendor Name  |                                |
| Vendor ID    |                                |
| Product Name | Basic device                   |
| Product ID   |                                |
| Granularity  | 8                              |
| RPDO count   | 4                              |
| TPDO count   | 4                              |
| LSS Slave    | True                           |
| LSS Master   | False                          |

#### Supported Baud rates
* [x] 10 kBit/s
* [x] 20 kBit/s
* [x] 50 kBit/s
* [x] 125 kBit/s
* [x] 250 kBit/s
* [x] 500 kBit/s
* [x] 800 kBit/s
* [x] 1000 kBit/s
* [ ] auto


PDO Mapping {#pdo-mapping}
--------------------------

### RPDO 0x1400
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x00000200+$NODEID                                            |
| Transmission | type=255                                                      |
|   0x62000108 | Write digital output 8-bit (Output)                           |
|   0x62000208 | Write digital output 8-bit (Output)                           |


### RPDO 0x1402
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x80000400+$NODEID                                            |
| Transmission | type=254                                                      |
|   0x64110110 | Write analog output 16-bit (Output)                           |
|   0x64110210 | Write analog output 16-bit (Output)                           |
|   0x64110310 | Write analog output 16-bit (Output)                           |
|   0x64110410 | Write analog output 16-bit (Output)                           |


### TPDO 0x1800
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x40000180+$NODEID                                            |
| Transmission | type=255; inhibit-time=0; event-timer=0                       |
|   0x60000108 | Read digital input 8-bit (Input)                              |
|   0x60000208 | Read digital input 8-bit (Input)                              |


### TPDO 0x1801
|              |                                                               |
| ------------ | ------------------------------------------------------------- |
| COB_ID       | 0x40000280+$NODEID                                            |
| Transmission | type=255; inhibit-time=0; event-timer=0                       |
|   0x64010110 | Read analog input 16-bit (Input)                              |
|   0x64010210 | Read analog input 16-bit (Input)                              |
|   0x64010310 | Read analog input 16-bit (Input)                              |
|   0x64010410 | Read analog input 16-bit (Input)                              |


Communication Specific Parameters {#communication-specific-parameters}
----------------------------------------------------------------------

### 0x1000 - Device type
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         | NMT            | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | ro  | no  | no   | 0x000F0191                      |

* bit 16-31: Additional information
* bit 0-15: Device profile number

### 0x1001 - Error register
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         | EM             | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED8               | ro  | tr  | no   | 0x00                            |

* bit 7: manufacturer specific
* bit 6: Reserved (always 0)
* bit 5: device profile specific
* bit 4: communication error (overrun, error state)
* bit 3: temperature
* bit 2: voltage
* bit 1: current
* bit 0: generic error

### 0x1003 - Pre-defined error field
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of errors      | UNSIGNED8  | rw  | no  | no   | 0             |
| 0x01 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x02 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x03 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x04 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x05 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x06 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x07 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x08 | Standard error field  | UNSIGNED32 | ro  | no  | no   | 0x00000000    |

* Sub Index 0: Contains number of actual errors. 0 can be written to clear error history.
* sub-index 1 and above:
  * bit 16-31: Manufacturer specific additional information
  * bit 0-15: Error code as transmited in the Emergency object

### 0x1005 - COB-ID SYNC message
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         | SYNC           | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0x00000080                      |

* bit 31: set to 0
* bit 30: If set, CANopen device generates SYNC object
* bit 11-29: set to 0
* bit 0-10: 11-bit CAN-ID

### 0x1006 - Communication cycle period
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         | SYNC_PROD      | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

Period of SYNC transmission in µs (0 = transmission disabled).

### 0x1007 - Synchronous window length
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0                               |

Synchronous window leghth in µs (0 = not used). All synchronous PDOs must be transmitted within this time window.

### 0x1010 - Store parameters
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       | STORAGE        | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x06          |
| 0x01 | Save all parameters   | UNSIGNED32 | rw  | no  | no   | 0x00000003    |
| 0x02 | Save communication parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x03 | Save application parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x04 | Save LSS parameters   | UNSIGNED32 | rw  | no  | no   | 0x00000003    |
| 0x05 | Save testing parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x06 | Save testing auto-store parameters| UNSIGNED32 | rw  | no  | no   | 0x00000003    |

Sub-indexes 1 and above:
* Reading provides information about its storage functionality:
  * bit 0: If set, CANopen device saves parameters on command
  * bit 1: If set, CANopen device saves parameters autonomously
* Writing value 0x65766173 ('s','a','v','e' from LSB to MSB) stores corresponding data.

### 0x1011 - Restore default parameters
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x06          |
| 0x01 | Restore all default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x02 | Restore communication default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x03 | Restore application default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x04 | Restore LSS default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x05 | Restore testing default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |
| 0x06 | Restore testing auto-store default parameters| UNSIGNED32 | rw  | no  | no   | 0x00000001    |

Sub-indexes 1 and above:
* Reading provides information about its restoring capability:
  * bit 0: If set, CANopen device restores parameters
* Writing value 0x64616F6C ('l','o','a','d' from LSB to MSB) restores corresponding data.

### 0x1012 - COB-ID time stamp object
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         | TIME           | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0x00000100                      |

* bit 31: If set, CANopen device consumes TIME message
* bit 30: If set, CANopen device produces TIME message
* bit 11-29: set to 0
* bit 0-10: 11-bit CAN-ID

### 0x1014 - COB-ID EMCY
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         | EM_PROD        | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | rw  | no  | no   | 0x80+$NODEID                    |

* bit 31: If set, EMCY does NOT exist / is NOT valid
* bit 11-30: set to 0
* bit 0-10: 11-bit CAN-ID

### 0x1015 - Inhibit time EMCY
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | rw  | no  | no   | 0                               |

Inhibit time of emergency message in multiples of 100µs. The value 0 disables the inhibit time.

### 0x1016 - Consumer heartbeat time
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       | HB_CONS        | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x08          |
| 0x01 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x02 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x03 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x04 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x05 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Consumer heartbeat time| UNSIGNED32 | rw  | no  | no   | 0x00000000    |

Consumer Heartbeat Time:
  * bit 24-31: set to 0
  * bit 16-23: Node ID of the monitored node. If 0 or greater than 127, sub-entry is not used.
  * bit 0-15: Heartbeat time in ms (if 0, sub-intry is not used). Value should be higher than the corresponding producer heartbeat time.

### 0x1017 - Producer heartbeat time
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         | HB_PROD        | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED16              | rw  | no  | no   | 0                               |

Heartbeat producer time in ms (0 = disable transmission).

### 0x1018 - Identity
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x04          |
| 0x01 | Vendor-ID             | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x02 | Product code          | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x03 | Revision number       | UNSIGNED32 | ro  | no  | no   | 0x00000000    |
| 0x04 | Serial number         | UNSIGNED32 | ro  | no  | no   | 0x00000000    |

* Vendor-ID, assigned by CiA
* Product code, manufacturer specific
* Revision number:
  * bit 16-31: Major revision number (CANopen behavior has changed)
  * bit 0-15: Minor revision num. (CANopen behavior has not changed)
* Serial number, manufacturer specific

### 0x1019 - Synchronous counter overflow value
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_COMM   |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED8               | rw  | no  | no   | 0                               |

* Value 0: SYNC message is transmitted with data length 0.
* Value 1: reserved.
* Value 2-240: SYNC message has one data byte, which contains the counter.
* Value 241-255: reserved.

### 0x1200 - SDO server parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | SDO_SRV        | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 2             |
| 0x01 | COB-ID client to server (rx)| UNSIGNED32 | ro  | t   | no   | 0x600+$NODEID |
| 0x02 | COB-ID server to client (tx)| UNSIGNED32 | ro  | t   | no   | 0x580+$NODEID |

Sub-indexes 1 and 2:
* bit 11-31: set to 0
* bit 0-10: 11-bit CAN-ID

### 0x1280 - SDO client parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | SDO_CLI        | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x03          |
| 0x01 | COB-ID client to server (tx)| UNSIGNED32 | rw  | tr  | no   | 0x80000000    |
| 0x02 | COB-ID server to client (rx)| UNSIGNED32 | rw  | tr  | no   | 0x80000000    |
| 0x03 | Node-ID of the SDO server| UNSIGNED8  | rw  | no  | no   | 0x01          |

* Sub-indexes 1 and 2:
  * bit 31: If set, SDO does NOT exist / is NOT valid
  * bit 30: If set, value is assigned dynamically
  * bit 11-29: set to 0
  * bit 0-10: 11-bit CAN-ID
* Node-ID of the SDO server, 0x01 to 0x7F

### 0x1400 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | RPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x05          |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x00000200+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 255           |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 11-30: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0-240: synchronous, processed after next reception of SYNC object
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Event timer in ms (0 = disabled) for deadline monitoring.

### 0x1401 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | RPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x05          |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x00000300+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 255           |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 11-30: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0-240: synchronous, processed after next reception of SYNC object
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Event timer in ms (0 = disabled) for deadline monitoring.

### 0x1402 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | RPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x05          |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x80000400+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 11-30: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0-240: synchronous, processed after next reception of SYNC object
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Event timer in ms (0 = disabled) for deadline monitoring.

### 0x1403 - RPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | RPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x05          |
| 0x01 | COB-ID used by RPDO   | UNSIGNED32 | rw  | no  | no   | 0x80000500+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 11-30: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0-240: synchronous, processed after next reception of SYNC object
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Event timer in ms (0 = disabled) for deadline monitoring.

### 0x1600 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 2             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x62000108    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x62000208    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x62000308    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x62000408    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x62000508    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x62000608    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x62000708    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x62000808    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

### 0x1601 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 0             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

### 0x1602 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 4             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x64110110    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x64110210    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x64110310    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x64110410    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

### 0x1603 - RPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 0             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

### 0x1800 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | TPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x06          |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0x40000180+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 255           |
| 0x03 | Inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 30: If set, NO RTR is allowed on this PDO
  * bit 11-29: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0: synchronous (acyclic)
  * Value 1-240: synchronous (cyclic every (1-240)-th sync)
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Inhibit time in multiple of 100µs, if the transmission type is set to 254 or 255 (0 = disabled).
* Event timer interval in ms, if the transmission type is set to 254 or 255 (0 = disabled).
* SYNC start value
  * Value 0: Counter of the SYNC message shall not be processed.
  * Value 1-240: The SYNC message with the counter value equal to this value shall be regarded as the first received SYNC message.

### 0x1801 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | TPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x06          |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0x40000280+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 255           |
| 0x03 | Inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 30: If set, NO RTR is allowed on this PDO
  * bit 11-29: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0: synchronous (acyclic)
  * Value 1-240: synchronous (cyclic every (1-240)-th sync)
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Inhibit time in multiple of 100µs, if the transmission type is set to 254 or 255 (0 = disabled).
* Event timer interval in ms, if the transmission type is set to 254 or 255 (0 = disabled).
* SYNC start value
  * Value 0: Counter of the SYNC message shall not be processed.
  * Value 1-240: The SYNC message with the counter value equal to this value shall be regarded as the first received SYNC message.

### 0x1802 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | TPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x06          |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0xC0000380+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x03 | Inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 30: If set, NO RTR is allowed on this PDO
  * bit 11-29: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0: synchronous (acyclic)
  * Value 1-240: synchronous (cyclic every (1-240)-th sync)
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Inhibit time in multiple of 100µs, if the transmission type is set to 254 or 255 (0 = disabled).
* Event timer interval in ms, if the transmission type is set to 254 or 255 (0 = disabled).
* SYNC start value
  * Value 0: Counter of the SYNC message shall not be processed.
  * Value 1-240: The SYNC message with the counter value equal to this value shall be regarded as the first received SYNC message.

### 0x1803 - TPDO communication parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      | TPDO           | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x06          |
| 0x01 | COB-ID used by TPDO   | UNSIGNED32 | rw  | no  | no   | 0xC0000480+$NODEID|
| 0x02 | Transmission type     | UNSIGNED8  | rw  | no  | no   | 254           |
| 0x03 | Inhibit time          | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x05 | Event timer           | UNSIGNED16 | rw  | no  | no   | 0             |
| 0x06 | SYNC start value      | UNSIGNED8  | rw  | no  | no   | 0             |

* COB-ID used by RPDO:
  * bit 31: If set, PDO does not exist / is not valid
  * bit 30: If set, NO RTR is allowed on this PDO
  * bit 11-29: set to 0
  * bit 0-10: 11-bit CAN-ID
* Transmission type:
  * Value 0: synchronous (acyclic)
  * Value 1-240: synchronous (cyclic every (1-240)-th sync)
  * Value 241-253: not used
  * Value 254: event-driven (manufacturer-specific)
  * Value 255: event-driven (device profile and application profile specific)
* Inhibit time in multiple of 100µs, if the transmission type is set to 254 or 255 (0 = disabled).
* Event timer interval in ms, if the transmission type is set to 254 or 255 (0 = disabled).
* SYNC start value
  * Value 0: Counter of the SYNC message shall not be processed.
  * Value 1-240: The SYNC message with the counter value equal to this value shall be regarded as the first received SYNC message.

### 0x1A00 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 2             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x60000108    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x60000208    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x60000308    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x60000408    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x60000508    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x60000608    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x60000708    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x60000808    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

### 0x1A01 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 4             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x64010110    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x64010210    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x64010310    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x64010410    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

### 0x1A02 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 0             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

### 0x1A03 - TPDO mapping parameter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_COMM   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Number of mapped application objects in PDO| UNSIGNED8  | rw  | no  | no   | 0             |
| 0x01 | Application object 1  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x02 | Application object 2  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x03 | Application object 3  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x04 | Application object 4  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x05 | Application object 5  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x06 | Application object 6  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x07 | Application object 7  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |
| 0x08 | Application object 8  | UNSIGNED32 | rw  | no  | no   | 0x00000000    |

* Number of mapped application objects in PDO:
  * Value 0: mapping is disabled.
  * Value 1: sub-index 0x01 is valid.
  * Value 2-8: sub-indexes 0x01 to (0x02 to 0x08) are valid.
* Application object 1-8:
  * bit 16-31: index
  * bit 8-15: sub-index
  * bit 0-7: data length in bits

Manufacturer Specific Parameters {#manufacturer-specific-parameters}
--------------------------------------------------------------------

### 0x2100 - Error status bits
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | RAM            |

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| OCTET_STRING            | ro  | t   | no   | 00 00 00 00 00 00 00 00 00 00   |

Error Status Bits indicates internal error conditions inside stack or inside application. For details see CO_EM_errorStatusBits_t in CO_Emergency.h file or in https://canopennode.github.io/CANopenSocket/group__CO__Emergency.html

### 0x2105 - Version
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x02          |
| 0x01 | CANopenNode           | VISIBLE_STRING| ro  | no  | no   |               |
| 0x02 | Application           | VISIBLE_STRING| ro  | no  | no   |               |

### 0x2106 - Power-on counter
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| VAR         |                | PERSIST_TEST_AUTO|

| Data Type               | SDO | PDO | SRDO | Default Value                   |
| ----------------------- | --- | --- | ---- | ------------------------------- |
| UNSIGNED32              | ro  | no  | no   | 0                               |

Power on Counter counts total microcontroller resets in it's lifetime. Variable is an example of EEPROM usage.

### 0x2110 - Variable Int32
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | tr  | no   | 0x10          |
| 0x01 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x02 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x03 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x04 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x05 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x06 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x07 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x08 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x09 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x0A | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x0B | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x0C | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x0D | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x0E | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x0F | int32                 | INTEGER32  | rw  | tr  | no   | 0             |
| 0x10 | int32                 | INTEGER32  | rw  | tr  | no   | 0             |

Vartiable is free to use by application.

### 0x2111 - Variable Int32 save
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | PERSIST_TEST   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | tr  | no   | 0x10          |
| 0x01 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x02 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x03 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x04 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x05 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x06 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x07 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x08 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x09 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0A | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0B | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0C | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0D | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0E | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0F | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x10 | int32                 | INTEGER32  | rw  | no  | no   | 0             |

Vartiable is free to use by application. Variables can be saved on command.

### 0x2112 - Variable NV Int32 auto save
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | PERSIST_TEST_AUTO|

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | tr  | no   | 0x10          |
| 0x01 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x02 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x03 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x04 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x05 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x06 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x07 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x08 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x09 | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0A | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0B | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0C | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0D | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0E | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x0F | int32                 | INTEGER32  | rw  | no  | no   | 0             |
| 0x10 | int32                 | INTEGER32  | rw  | no  | no   | 0             |

Vartiable is free to use by application. Variable is automatically saved.

### 0x2120 - Testing variables
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| RECORD      |                | PERSIST_TEST   |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x0C          |
| 0x01 | I64                   | INTEGER64  | rw  | tr  | no   | -1234567890123456789|
| 0x02 | U64                   | UNSIGNED64 | rw  | tr  | no   | 0x1234567890ABCDEF|
| 0x03 | R32                   | REAL32     | rw  | tr  | no   | 12.345        |
| 0x04 | R64                   | REAL64     | rw  | tr  | no   | 456.789       |
| 0x05 | Average of four numbers| REAL64     | ro  | t   | no   |               |
| 0x06 | String short          | VISIBLE_STRING| rw  | no  | no   | str           |
| 0x07 | String long           | VISIBLE_STRING (len=1000)| rw  | no  | no   | 1000 bytes long string buffer. Visible string may contain UTF-8 characters, like this 3 bytes long '€' Euro symbol. It may contain also control characters like tabs '	' or newLines '
'.|
| 0x08 | Octet string          | OCTET_STRING| rw  | no  | no   | C8 3D BB      |
| 0x09 | Parameter with default value| UNSIGNED16 | rw  | no  | no   | 0x1234        |
| 0x0A | Domain                | DOMAIN     | rw  | no  | no   |               |
| 0x0B | Domain file name read | VISIBLE_STRING (len=100)| rw  | no  | no   | basicDevice.md|
| 0x0C | Domain file name write| VISIBLE_STRING (len=100)| rw  | no  | no   | fileWrittenByDomain|

Device Profile Specific Parameters {#device-profile-specific-parameters}
------------------------------------------------------------------------

### 0x6000 - Read digital input 8-bit
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | no  | no   | 0x08          |
| 0x01 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |
| 0x02 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |
| 0x03 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |
| 0x04 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |
| 0x05 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |
| 0x06 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |
| 0x07 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |
| 0x08 | Input                 | UNSIGNED8  | ro  | t   | no   | 0x00          |

Digital inputs from hardware.

### 0x6200 - Write digital output 8-bit
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | tr  | no   | 0x08          |
| 0x01 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |
| 0x02 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |
| 0x03 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |
| 0x04 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |
| 0x05 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |
| 0x06 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |
| 0x07 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |
| 0x08 | Output                | UNSIGNED8  | rw  | tr  | no   | 0x00          |

Digital outputs on hardware.

### 0x6401 - Read analog input 16-bit
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | tr  | no   | 0x10          |
| 0x01 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x02 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x03 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x04 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x05 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x06 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x07 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x08 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x09 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x0A | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x0B | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x0C | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x0D | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x0E | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x0F | Input                 | INTEGER16  | ro  | tr  | no   | 0             |
| 0x10 | Input                 | INTEGER16  | ro  | tr  | no   | 0             |

Analogue inputs from hardware. The integer value is left adjusted.

### 0x6411 - Write analog output 16-bit
| Object Type | Count Label    | Storage Group  |
| ----------- | -------------- | -------------- |
| ARRAY       |                | RAM            |

| Sub  | Name                  | Data Type  | SDO | PDO | SRDO | Default Value |
| ---- | --------------------- | ---------- | --- | --- | ---- | ------------- |
| 0x00 | Highest sub-index supported| UNSIGNED8  | ro  | tr  | no   | 0x08          |
| 0x01 | Output                | INTEGER16  | rw  | tr  | no   | 0             |
| 0x02 | Output                | INTEGER16  | rw  | tr  | no   | 0             |
| 0x03 | Output                | INTEGER16  | rw  | tr  | no   | 0             |
| 0x04 | Output                | INTEGER16  | rw  | tr  | no   | 0             |
| 0x05 | Output                | INTEGER16  | rw  | tr  | no   | 0             |
| 0x06 | Output                | INTEGER16  | rw  | tr  | no   | 0             |
| 0x07 | Output                | INTEGER16  | rw  | tr  | no   | 0             |
| 0x08 | Output                | INTEGER16  | rw  | tr  | no   | 0             |

Analogue outputs on hardware. The integer value is left adjusted.
