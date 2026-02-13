/*******************************************************************************
** File: bmp280_app_msg.h
**
** Purpose:
**   This file contains the message definitions for the BMP280 application.
**
*******************************************************************************/
#ifndef BMP280_APP_MSG_H
#define BMP280_APP_MSG_H

/*
** BMP280 App command codes
*/
#define BMP280_APP_NOOP_CC           0
#define BMP280_APP_RESET_COUNTERS_CC 1
#define BMP280_APP_READ_SENSOR_CC    2

/*
** Event message ID counts
*/
#define BMP280_APP_EVENT_COUNTS 12

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
} BMP280_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
} BMP280_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
} BMP280_ReadSensorCmd_t;

/*
** Housekeeping telemetry packet
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8  CommandCounter;
    uint8  CommandErrorCounter;
    uint8  SensorStatus;  /* 0=Not initialized, 1=Initialized, 2=Error */
    uint8  spare[1];
} BMP280_HkTlm_t;

/*
** Sensor data telemetry packet
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    float  Temperature;      /* Temperature in degrees Celsius */
    float  Pressure;         /* Pressure in Pascals */
    uint32 ReadCounter;      /* Number of successful reads */
    uint32 ErrorCounter;     /* Number of read errors */
} BMP280_SensorTlm_t;

#endif /* BMP280_APP_MSG_H */
