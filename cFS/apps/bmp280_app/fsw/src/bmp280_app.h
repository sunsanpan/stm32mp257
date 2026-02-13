/*******************************************************************************
** File: bmp280_app.h
**
** Purpose:
**   This file contains the main definitions and declarations for the BMP280
**   temperature and pressure sensor application for cFS on STM32MP257F-EV1.
**
*******************************************************************************/
#ifndef BMP280_APP_H
#define BMP280_APP_H

/*
** Required header files
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include "bmp280_app_perfids.h"
#include "bmp280_app_msgids.h"
#include "bmp280_app_msg.h"

/*
** BMP280 I2C Configuration
*/
#define BMP280_I2C_BUS          "/dev/i2c-1"  /* I2C bus device */
#define BMP280_I2C_ADDR         0x76          /* BMP280 I2C address (can be 0x77) */

/*
** BMP280 Register Addresses
*/
#define BMP280_REG_DIG_T1       0x88
#define BMP280_REG_DIG_T2       0x8A
#define BMP280_REG_DIG_T3       0x8C
#define BMP280_REG_DIG_P1       0x8E
#define BMP280_REG_DIG_P2       0x90
#define BMP280_REG_DIG_P3       0x92
#define BMP280_REG_DIG_P4       0x94
#define BMP280_REG_DIG_P5       0x96
#define BMP280_REG_DIG_P6       0x98
#define BMP280_REG_DIG_P7       0x9A
#define BMP280_REG_DIG_P8       0x9C
#define BMP280_REG_DIG_P9       0x9E
#define BMP280_REG_CHIPID       0xD0
#define BMP280_REG_VERSION      0xD1
#define BMP280_REG_SOFTRESET    0xE0
#define BMP280_REG_CONTROL      0xF4
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_PRESS_MSB    0xF7
#define BMP280_REG_PRESS_LSB    0xF8
#define BMP280_REG_PRESS_XLSB   0xF9
#define BMP280_REG_TEMP_MSB     0xFA
#define BMP280_REG_TEMP_LSB     0xFB
#define BMP280_REG_TEMP_XLSB    0xFC

/*
** BMP280 Chip ID
*/
#define BMP280_CHIP_ID          0x58

/*
** BMP280 Calibration Data Structure
*/
typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    int32_t  t_fine;  /* Temperature fine value for compensation */
} BMP280_CalibData_t;

/*
** Application Global Data Structure
*/
typedef struct
{
    /*
    ** Command interface counters
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet
    */
    BMP280_HkTlm_t HkTlm;

    /*
    ** Sensor data telemetry packet
    */
    BMP280_SensorTlm_t SensorTlm;

    /*
    ** Run status variable
    */
    uint32 RunStatus;

    /*
    ** Operational data
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Event table ID for event message filtering
    */
    CFE_EVS_BinFilter_t EventFilters[BMP280_APP_EVENT_COUNTS];

    /*
    ** I2C file descriptor
    */
    int32 I2C_fd;

    /*
    ** BMP280 calibration data
    */
    BMP280_CalibData_t CalibData;

    /*
    ** Sensor status
    */
    bool SensorInitialized;

} BMP280_AppData_t;

/*
** Function Prototypes
*/
void  BMP280_APP_Main(void);
int32 BMP280_APP_Init(void);
void  BMP280_APP_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr);
void  BMP280_APP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr);
int32 BMP280_APP_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg);
int32 BMP280_APP_ResetCounters(const BMP280_ResetCountersCmd_t *Msg);
int32 BMP280_APP_Noop(const BMP280_NoopCmd_t *Msg);
int32 BMP280_APP_ReadSensor(const BMP280_ReadSensorCmd_t *Msg);
bool  BMP280_APP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

/*
** BMP280 Hardware Interface Functions
*/
int32 BMP280_InitI2C(void);
int32 BMP280_CloseI2C(void);
int32 BMP280_WriteRegister(uint8 reg, uint8 value);
int32 BMP280_ReadRegister(uint8 reg, uint8 *value);
int32 BMP280_ReadRegisters(uint8 reg, uint8 *buffer, uint8 length);
int32 BMP280_InitSensor(void);
int32 BMP280_ReadCalibration(void);
int32 BMP280_ReadRawData(int32 *adc_T, int32 *adc_P);
float BMP280_CompensateTemperature(int32 adc_T);
float BMP280_CompensatePressure(int32 adc_P);

#endif /* BMP280_APP_H */
