/*******************************************************************************
    CANopen Object Dictionary definition for CANopenNode V4

    This file was automatically generated with
    libedssharp Object Dictionary Editor v0.8-109-gb3f85db

    https://github.com/CANopenNode/CANopenNode
    https://github.com/robincornelius/libedssharp

    DON'T EDIT THIS FILE MANUALLY !!!!
********************************************************************************

    File info:
        File Names:   OD.h; OD.c
        Project File: basicDevice.xdd
        File Version: 1

        Created:      28. 11. 2020 12:37:54
        Created By:   Janez Paternoster
        Modified:     23. 12. 2020 12:22:00
        Modified By:  Janez Paternoster

    Device Info:
        Vendor Name:  
        Vendor ID:    
        Product Name: Basic device
        Product ID:   

        Description:  Basic CANopen device with example usage.
*******************************************************************************/

#ifndef OD_H
#define OD_H
/*******************************************************************************
    Counters of OD objects
*******************************************************************************/
#define OD_CNT_NMT 1
#define OD_CNT_EM 1
#define OD_CNT_SYNC 1
#define OD_CNT_SYNC_PROD 1
#define OD_CNT_TIME 1
#define OD_CNT_EM_PROD 1
#define OD_CNT_HB_CONS 1
#define OD_CNT_HB_PROD 1
#define OD_CNT_SDO_SRV 1
#define OD_CNT_SDO_CLI 1
#define OD_CNT_RPDO 4
#define OD_CNT_TPDO 4


/*******************************************************************************
    OD data declaration of all groups
*******************************************************************************/
typedef struct {
    uint32_t x1000_deviceType;
    uint32_t x1005_COB_ID_SYNCMessage;
    uint32_t x1006_communicationCyclePeriod;
    uint32_t x1007_synchronousWindowLength;
    uint32_t x1012_COB_IDTimeStampObject;
    uint32_t x1014_COB_ID_EMCY;
    uint16_t x1015_inhibitTimeEMCY;
    uint8_t x1016_consumerHeartbeatTime_sub0;
    uint32_t x1016_consumerHeartbeatTime[8];
    uint16_t x1017_producerHeartbeatTime;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t vendor_ID;
        uint32_t productCode;
        uint32_t revisionNumber;
        uint32_t serialNumber;
    } x1018_identity;
    uint8_t x1019_synchronousCounterOverflowValue;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDClientToServerTx;
        uint32_t COB_IDServerToClientRx;
        uint8_t node_IDOfTheSDOServer;
    } x1280_SDOClientParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByRPDO;
        uint8_t transmissionType;
        uint16_t eventTimer;
    } x1400_RPDOCommunicationParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByRPDO;
        uint8_t transmissionType;
        uint16_t eventTimer;
    } x1401_RPDOCommunicationParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByRPDO;
        uint8_t transmissionType;
        uint16_t eventTimer;
    } x1402_RPDOCommunicationParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByRPDO;
        uint8_t transmissionType;
        uint16_t eventTimer;
    } x1403_RPDOCommunicationParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1600_RPDOMappingParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1601_RPDOMappingParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1602_RPDOMappingParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1603_RPDOMappingParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByTPDO;
        uint8_t transmissionType;
        uint16_t inhibitTime;
        uint16_t eventTimer;
        uint8_t SYNCStartValue;
    } x1800_TPDOCommunicationParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByTPDO;
        uint8_t transmissionType;
        uint16_t inhibitTime;
        uint16_t eventTimer;
        uint8_t SYNCStartValue;
    } x1801_TPDOCommunicationParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByTPDO;
        uint8_t transmissionType;
        uint16_t inhibitTime;
        uint16_t eventTimer;
        uint8_t SYNCStartValue;
    } x1802_TPDOCommunicationParameter;
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDUsedByTPDO;
        uint8_t transmissionType;
        uint16_t inhibitTime;
        uint16_t eventTimer;
        uint8_t SYNCStartValue;
    } x1803_TPDOCommunicationParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1A00_TPDOMappingParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1A01_TPDOMappingParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1A02_TPDOMappingParameter;
    struct {
        uint8_t numberOfMappedApplicationObjectsInPDO;
        uint32_t applicationObject_1;
        uint32_t applicationObject_2;
        uint32_t applicationObject_3;
        uint32_t applicationObject_4;
        uint32_t applicationObject_5;
        uint32_t applicationObject_6;
        uint32_t applicationObject_7;
        uint32_t applicationObject_8;
    } x1A03_TPDOMappingParameter;
} OD_PERSIST_COMM_t;

typedef struct {
    uint8_t x1001_errorRegister;
    uint8_t x1003_pre_definedErrorField_sub0;
    uint32_t x1003_pre_definedErrorField[8];
    uint8_t x1010_storeParameters_sub0;
    uint32_t x1010_storeParameters[4];
    uint8_t x1011_restoreDefaultParameters_sub0;
    uint32_t x1011_restoreDefaultParameters[4];
    struct {
        uint8_t highestSub_indexSupported;
        uint32_t COB_IDClientToServerRx;
        uint32_t COB_IDServerToClientTx;
    } x1200_SDOServerParameter;
    uint8_t x2100_errorStatusBits[10];
    struct {
        uint8_t highestSub_indexSupported;
    } x2105_version;
    uint8_t x2110_variableInt32_sub0;
    int32_t x2110_variableInt32[16];
    uint8_t x6000_readDigitalInput_8_bit_sub0;
    uint8_t x6000_readDigitalInput_8_bit[8];
    uint8_t x6200_writeDigitalOutput_8_bit_sub0;
    uint8_t x6200_writeDigitalOutput_8_bit[8];
    uint8_t x6401_readAnalogInput_16_bit_sub0;
    int16_t x6401_readAnalogInput_16_bit[16];
    uint8_t x6411_writeAnalogOutput_16_bit_sub0;
    int16_t x6411_writeAnalogOutput_16_bit[8];
} OD_RAM_t;

typedef struct {
    uint32_t x2106_power_onCounter;
    uint8_t x2112_variableNV_Int32_autoSave_sub0;
    int32_t x2112_variableNV_Int32_autoSave[16];
} OD_PERSIST_APP_AUTO_t;

typedef struct {
    uint8_t x2111_variableInt32_save_sub0;
    int32_t x2111_variableInt32_save[16];
    struct {
        uint8_t highestSub_indexSupported;
        int64_t I64;
        uint64_t U64;
        float32_t R32;
        float64_t R64;
        char stringShort[4];
        char stringLong[1001];
        uint8_t octetString[10];
        uint16_t parameterWithDefaultValue;
        char domainFileNameRead[101];
        char domainFileNameWrite[101];
    } x2120_testingVariables;
} OD_PERSIST_APP_t;

extern OD_PERSIST_COMM_t OD_PERSIST_COMM;
extern OD_RAM_t OD_RAM;
extern OD_PERSIST_APP_AUTO_t OD_PERSIST_APP_AUTO;
extern OD_PERSIST_APP_t OD_PERSIST_APP;
extern const OD_t OD;


/*******************************************************************************
    Object dictionary entries - shortcuts
*******************************************************************************/
#define OD_ENTRY_H1000 &OD.list[0]
#define OD_ENTRY_H1001 &OD.list[1]
#define OD_ENTRY_H1003 &OD.list[2]
#define OD_ENTRY_H1005 &OD.list[3]
#define OD_ENTRY_H1006 &OD.list[4]
#define OD_ENTRY_H1007 &OD.list[5]
#define OD_ENTRY_H1010 &OD.list[6]
#define OD_ENTRY_H1011 &OD.list[7]
#define OD_ENTRY_H1012 &OD.list[8]
#define OD_ENTRY_H1014 &OD.list[9]
#define OD_ENTRY_H1015 &OD.list[10]
#define OD_ENTRY_H1016 &OD.list[11]
#define OD_ENTRY_H1017 &OD.list[12]
#define OD_ENTRY_H1018 &OD.list[13]
#define OD_ENTRY_H1019 &OD.list[14]
#define OD_ENTRY_H1200 &OD.list[15]
#define OD_ENTRY_H1280 &OD.list[16]
#define OD_ENTRY_H1400 &OD.list[17]
#define OD_ENTRY_H1401 &OD.list[18]
#define OD_ENTRY_H1402 &OD.list[19]
#define OD_ENTRY_H1403 &OD.list[20]
#define OD_ENTRY_H1600 &OD.list[21]
#define OD_ENTRY_H1601 &OD.list[22]
#define OD_ENTRY_H1602 &OD.list[23]
#define OD_ENTRY_H1603 &OD.list[24]
#define OD_ENTRY_H1800 &OD.list[25]
#define OD_ENTRY_H1801 &OD.list[26]
#define OD_ENTRY_H1802 &OD.list[27]
#define OD_ENTRY_H1803 &OD.list[28]
#define OD_ENTRY_H1A00 &OD.list[29]
#define OD_ENTRY_H1A01 &OD.list[30]
#define OD_ENTRY_H1A02 &OD.list[31]
#define OD_ENTRY_H1A03 &OD.list[32]
#define OD_ENTRY_H2100 &OD.list[33]
#define OD_ENTRY_H2105 &OD.list[34]
#define OD_ENTRY_H2106 &OD.list[35]
#define OD_ENTRY_H2110 &OD.list[36]
#define OD_ENTRY_H2111 &OD.list[37]
#define OD_ENTRY_H2112 &OD.list[38]
#define OD_ENTRY_H2120 &OD.list[39]
#define OD_ENTRY_H6000 &OD.list[40]
#define OD_ENTRY_H6200 &OD.list[41]
#define OD_ENTRY_H6401 &OD.list[42]
#define OD_ENTRY_H6411 &OD.list[43]


/*******************************************************************************
    Object dictionary entries - shortcuts with names
*******************************************************************************/
#define OD_ENTRY_H1000_deviceType &OD.list[0]
#define OD_ENTRY_H1001_errorRegister &OD.list[1]
#define OD_ENTRY_H1003_pre_definedErrorField &OD.list[2]
#define OD_ENTRY_H1005_COB_ID_SYNCMessage &OD.list[3]
#define OD_ENTRY_H1006_communicationCyclePeriod &OD.list[4]
#define OD_ENTRY_H1007_synchronousWindowLength &OD.list[5]
#define OD_ENTRY_H1010_storeParameters &OD.list[6]
#define OD_ENTRY_H1011_restoreDefaultParameters &OD.list[7]
#define OD_ENTRY_H1012_COB_IDTimeStampObject &OD.list[8]
#define OD_ENTRY_H1014_COB_ID_EMCY &OD.list[9]
#define OD_ENTRY_H1015_inhibitTimeEMCY &OD.list[10]
#define OD_ENTRY_H1016_consumerHeartbeatTime &OD.list[11]
#define OD_ENTRY_H1017_producerHeartbeatTime &OD.list[12]
#define OD_ENTRY_H1018_identity &OD.list[13]
#define OD_ENTRY_H1019_synchronousCounterOverflowValue &OD.list[14]
#define OD_ENTRY_H1200_SDOServerParameter &OD.list[15]
#define OD_ENTRY_H1280_SDOClientParameter &OD.list[16]
#define OD_ENTRY_H1400_RPDOCommunicationParameter &OD.list[17]
#define OD_ENTRY_H1401_RPDOCommunicationParameter &OD.list[18]
#define OD_ENTRY_H1402_RPDOCommunicationParameter &OD.list[19]
#define OD_ENTRY_H1403_RPDOCommunicationParameter &OD.list[20]
#define OD_ENTRY_H1600_RPDOMappingParameter &OD.list[21]
#define OD_ENTRY_H1601_RPDOMappingParameter &OD.list[22]
#define OD_ENTRY_H1602_RPDOMappingParameter &OD.list[23]
#define OD_ENTRY_H1603_RPDOMappingParameter &OD.list[24]
#define OD_ENTRY_H1800_TPDOCommunicationParameter &OD.list[25]
#define OD_ENTRY_H1801_TPDOCommunicationParameter &OD.list[26]
#define OD_ENTRY_H1802_TPDOCommunicationParameter &OD.list[27]
#define OD_ENTRY_H1803_TPDOCommunicationParameter &OD.list[28]
#define OD_ENTRY_H1A00_TPDOMappingParameter &OD.list[29]
#define OD_ENTRY_H1A01_TPDOMappingParameter &OD.list[30]
#define OD_ENTRY_H1A02_TPDOMappingParameter &OD.list[31]
#define OD_ENTRY_H1A03_TPDOMappingParameter &OD.list[32]
#define OD_ENTRY_H2100_errorStatusBits &OD.list[33]
#define OD_ENTRY_H2105_version &OD.list[34]
#define OD_ENTRY_H2106_power_onCounter &OD.list[35]
#define OD_ENTRY_H2110_variableInt32 &OD.list[36]
#define OD_ENTRY_H2111_variableInt32_save &OD.list[37]
#define OD_ENTRY_H2112_variableNV_Int32_autoSave &OD.list[38]
#define OD_ENTRY_H2120_testingVariables &OD.list[39]
#define OD_ENTRY_H6000_readDigitalInput_8_bit &OD.list[40]
#define OD_ENTRY_H6200_writeDigitalOutput_8_bit &OD.list[41]
#define OD_ENTRY_H6401_readAnalogInput_16_bit &OD.list[42]
#define OD_ENTRY_H6411_writeAnalogOutput_16_bit &OD.list[43]

#endif /* OD_H */
