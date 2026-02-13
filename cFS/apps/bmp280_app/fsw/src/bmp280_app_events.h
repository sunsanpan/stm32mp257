/*******************************************************************************
** File: bmp280_app_events.h
**
** Purpose:
**   This file contains the event ID definitions for the BMP280 application.
**
*******************************************************************************/
#ifndef BMP280_APP_EVENTS_H
#define BMP280_APP_EVENTS_H

/*
** Event IDs
*/
#define BMP280_RESERVED_EID          0
#define BMP280_STARTUP_INF_EID       1
#define BMP280_COMMAND_ERR_EID       2
#define BMP280_COMMANDNOP_INF_EID    3
#define BMP280_COMMANDRST_INF_EID    4
#define BMP280_INVALID_MSGID_ERR_EID 5
#define BMP280_LEN_ERR_EID           6
#define BMP280_PIPE_ERR_EID          7
#define BMP280_I2C_OPEN_ERR_EID      8
#define BMP280_I2C_INIT_ERR_EID      9
#define BMP280_SENSOR_READ_INF_EID   10
#define BMP280_SENSOR_READ_ERR_EID   11

#endif /* BMP280_APP_EVENTS_H */
