/*******************************************************************************
** File: bmp280_app.c
**
** Purpose:
**   This file contains the main source code for the BMP280 temperature and
**   pressure sensor application for cFS on STM32MP257F-EV1.
**
*******************************************************************************/

/*
** Include Files
*/
#include "bmp280_app.h"
#include "bmp280_app_events.h"
#include "bmp280_app_version.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

/*
** Global Data
*/
BMP280_AppData_t BMP280_AppData;

/*
** Application entry point and main process loop
*/
void BMP280_APP_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(BMP280_APP_MAIN_TASK_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the initialization fails, set the RunStatus to CFE_ES_RunStatus_APP_ERROR
    ** and the App will exit
    */
    status = BMP280_APP_Init();
    if (status != CFE_SUCCESS)
    {
        BMP280_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Application main loop
    */
    while (CFE_ES_RunLoop(&BMP280_AppData.RunStatus) == true)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(BMP280_APP_MAIN_TASK_PERF_ID);

        /*
        ** Wait for the next Software Bus message
        */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, BMP280_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance log entry stamp
        */
        CFE_ES_PerfLogEntry(BMP280_APP_MAIN_TASK_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            BMP280_APP_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(BMP280_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BMP280 APP: SB Pipe Read Error, App will exit. RC = 0x%08lX",
                              (unsigned long)status);

            BMP280_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Close I2C before exit
    */
    BMP280_CloseI2C();

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(BMP280_APP_MAIN_TASK_PERF_ID);

    CFE_ES_ExitApp(BMP280_AppData.RunStatus);
}

/*
** Initialize the application
*/
int32 BMP280_APP_Init(void)
{
    int32 status;

    BMP280_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    BMP280_AppData.CmdCounter = 0;
    BMP280_AppData.ErrCounter = 0;
    BMP280_AppData.SensorInitialized = false;
    BMP280_AppData.I2C_fd = -1;

    /*
    ** Initialize app configuration data
    */
    memset(&BMP280_AppData.CalibData, 0, sizeof(BMP280_CalibData_t));

    /*
    ** Initialize event filter table
    */
    BMP280_AppData.EventFilters[0].EventID = BMP280_STARTUP_INF_EID;
    BMP280_AppData.EventFilters[0].Mask    = 0x0000;
    BMP280_AppData.EventFilters[1].EventID = BMP280_COMMAND_ERR_EID;
    BMP280_AppData.EventFilters[1].Mask    = 0x0000;
    BMP280_AppData.EventFilters[2].EventID = BMP280_COMMANDNOP_INF_EID;
    BMP280_AppData.EventFilters[2].Mask    = 0x0000;
    BMP280_AppData.EventFilters[3].EventID = BMP280_COMMANDRST_INF_EID;
    BMP280_AppData.EventFilters[3].Mask    = 0x0000;
    BMP280_AppData.EventFilters[4].EventID = BMP280_INVALID_MSGID_ERR_EID;
    BMP280_AppData.EventFilters[4].Mask    = 0x0000;
    BMP280_AppData.EventFilters[5].EventID = BMP280_LEN_ERR_EID;
    BMP280_AppData.EventFilters[5].Mask    = 0x0000;
    BMP280_AppData.EventFilters[6].EventID = BMP280_PIPE_ERR_EID;
    BMP280_AppData.EventFilters[6].Mask    = 0x0000;
    BMP280_AppData.EventFilters[7].EventID = BMP280_I2C_OPEN_ERR_EID;
    BMP280_AppData.EventFilters[7].Mask    = 0x0000;
    BMP280_AppData.EventFilters[8].EventID = BMP280_I2C_INIT_ERR_EID;
    BMP280_AppData.EventFilters[8].Mask    = 0x0000;
    BMP280_AppData.EventFilters[9].EventID = BMP280_SENSOR_READ_INF_EID;
    BMP280_AppData.EventFilters[9].Mask    = 0x0000;
    BMP280_AppData.EventFilters[10].EventID = BMP280_SENSOR_READ_ERR_EID;
    BMP280_AppData.EventFilters[10].Mask   = 0x0000;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(BMP280_AppData.EventFilters, BMP280_APP_EVENT_COUNTS,
                              CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("BMP280 App: Error Registering Events, RC = 0x%08lX\n",
                             (unsigned long)status);
        return status;
    }

    /*
    ** Initialize housekeeping packet (clear user data area)
    */
    CFE_MSG_Init(CFE_MSG_PTR(BMP280_AppData.HkTlm.TlmHeader), CFE_SB_ValueToMsgId(BMP280_APP_HK_TLM_MID),
                 sizeof(BMP280_AppData.HkTlm));

    /*
    ** Initialize sensor data packet
    */
    CFE_MSG_Init(CFE_MSG_PTR(BMP280_AppData.SensorTlm.TlmHeader),
                 CFE_SB_ValueToMsgId(BMP280_APP_SENSOR_TLM_MID), sizeof(BMP280_AppData.SensorTlm));

    /*
    ** Create Software Bus message pipe
    */
    status = CFE_SB_CreatePipe(&BMP280_AppData.CommandPipe, 32, "BMP280_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("BMP280 App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(BMP280_APP_SEND_HK_MID), BMP280_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("BMP280 App: Error Subscribing to HK request, RC = 0x%08lX\n",
                             (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(BMP280_APP_CMD_MID), BMP280_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("BMP280 App: Error Subscribing to Command, RC = 0x%08lX\n",
                             (unsigned long)status);
        return status;
    }

    /*
    ** Initialize I2C and BMP280 sensor
    */
    status = BMP280_InitI2C();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(BMP280_I2C_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280 APP: Failed to initialize I2C");
        BMP280_AppData.HkTlm.SensorStatus = 2;  /* Error status */
    }
    else
    {
        status = BMP280_InitSensor();
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(BMP280_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BMP280 APP: Failed to initialize sensor");
            BMP280_AppData.HkTlm.SensorStatus = 2;  /* Error status */
        }
        else
        {
            BMP280_AppData.SensorInitialized = true;
            BMP280_AppData.HkTlm.SensorStatus = 1;  /* Initialized status */
        }
    }

    CFE_EVS_SendEvent(BMP280_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BMP280 APP Initialized. Version %d.%d.%d.%d", BMP280_APP_MAJOR_VERSION,
                      BMP280_APP_MINOR_VERSION, BMP280_APP_REVISION, BMP280_APP_MISSION_REV);

    return CFE_SUCCESS;
}

/*
** Process packets received on the command pipe
*/
void BMP280_APP_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case BMP280_APP_CMD_MID:
            BMP280_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case BMP280_APP_SEND_HK_MID:
            BMP280_APP_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(BMP280_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BMP280: invalid command packet, MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}

/*
** Process ground commands
*/
void BMP280_APP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case BMP280_APP_NOOP_CC:
            if (BMP280_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BMP280_NoopCmd_t)))
            {
                BMP280_APP_Noop((BMP280_NoopCmd_t *)SBBufPtr);
            }
            break;

        case BMP280_APP_RESET_COUNTERS_CC:
            if (BMP280_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BMP280_ResetCountersCmd_t)))
            {
                BMP280_APP_ResetCounters((BMP280_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case BMP280_APP_READ_SENSOR_CC:
            if (BMP280_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BMP280_ReadSensorCmd_t)))
            {
                BMP280_APP_ReadSensor((BMP280_ReadSensorCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(BMP280_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ground command code: CC = %d", CommandCode);
            BMP280_AppData.ErrCounter++;
            break;
    }
}

/*
** Report housekeeping telemetry
*/
int32 BMP280_APP_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{
    BMP280_AppData.HkTlm.CommandCounter = BMP280_AppData.CmdCounter;
    BMP280_AppData.HkTlm.CommandErrorCounter = BMP280_AppData.ErrCounter;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BMP280_AppData.HkTlm.TlmHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(BMP280_AppData.HkTlm.TlmHeader), true);

    return CFE_SUCCESS;
}

/*
** Reset counters command
*/
int32 BMP280_APP_ResetCounters(const BMP280_ResetCountersCmd_t *Msg)
{
    BMP280_AppData.CmdCounter = 0;
    BMP280_AppData.ErrCounter = 0;
    BMP280_AppData.SensorTlm.ReadCounter = 0;
    BMP280_AppData.SensorTlm.ErrorCounter = 0;

    CFE_EVS_SendEvent(BMP280_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BMP280: RESET command");

    return CFE_SUCCESS;
}

/*
** No-op command
*/
int32 BMP280_APP_Noop(const BMP280_NoopCmd_t *Msg)
{
    BMP280_AppData.CmdCounter++;

    CFE_EVS_SendEvent(BMP280_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BMP280: NOOP command. Version %d.%d.%d.%d", BMP280_APP_MAJOR_VERSION,
                      BMP280_APP_MINOR_VERSION, BMP280_APP_REVISION, BMP280_APP_MISSION_REV);

    return CFE_SUCCESS;
}

/*
** Read sensor command
*/
int32 BMP280_APP_ReadSensor(const BMP280_ReadSensorCmd_t *Msg)
{
    int32 status;
    int32 adc_T, adc_P;
    float temperature, pressure;

    BMP280_AppData.CmdCounter++;

    if (!BMP280_AppData.SensorInitialized)
    {
        CFE_EVS_SendEvent(BMP280_SENSOR_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Cannot read sensor - not initialized");
        BMP280_AppData.ErrCounter++;
        BMP280_AppData.SensorTlm.ErrorCounter++;
        return CFE_SUCCESS;
    }

    /*
    ** Start performance monitoring
    */
    CFE_ES_PerfLogEntry(BMP280_APP_SENSOR_READ_PERF_ID);

    /*
    ** Read raw sensor data
    */
    status = BMP280_ReadRawData(&adc_T, &adc_P);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(BMP280_SENSOR_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Failed to read raw sensor data");
        BMP280_AppData.ErrCounter++;
        BMP280_AppData.SensorTlm.ErrorCounter++;
        CFE_ES_PerfLogExit(BMP280_APP_SENSOR_READ_PERF_ID);
        return CFE_SUCCESS;
    }

    /*
    ** Compensate temperature and pressure
    */
    temperature = BMP280_CompensateTemperature(adc_T);
    pressure = BMP280_CompensatePressure(adc_P);

    /*
    ** Update telemetry
    */
    BMP280_AppData.SensorTlm.Temperature = temperature;
    BMP280_AppData.SensorTlm.Pressure = pressure;
    BMP280_AppData.SensorTlm.ReadCounter++;

    /*
    ** Send telemetry
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BMP280_AppData.SensorTlm.TlmHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(BMP280_AppData.SensorTlm.TlmHeader), true);

    CFE_EVS_SendEvent(BMP280_SENSOR_READ_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BMP280: Temp=%.2f C, Press=%.2f Pa", temperature, pressure);

    CFE_ES_PerfLogExit(BMP280_APP_SENSOR_READ_PERF_ID);

    return CFE_SUCCESS;
}

/*
** Verify command length
*/
bool BMP280_APP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(BMP280_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode,
                          (unsigned int)ActualLength, (unsigned int)ExpectedLength);

        result = false;
        BMP280_AppData.ErrCounter++;
    }

    return result;
}

/********************************************************************************
** BMP280 Hardware Interface Functions
********************************************************************************/

/*
** Initialize I2C interface
*/
int32 BMP280_InitI2C(void)
{
    BMP280_AppData.I2C_fd = open(BMP280_I2C_BUS, O_RDWR);
    if (BMP280_AppData.I2C_fd < 0)
    {
        CFE_EVS_SendEvent(BMP280_I2C_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Failed to open I2C bus %s", BMP280_I2C_BUS);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (ioctl(BMP280_AppData.I2C_fd, I2C_SLAVE, BMP280_I2C_ADDR) < 0)
    {
        close(BMP280_AppData.I2C_fd);
        BMP280_AppData.I2C_fd = -1;
        CFE_EVS_SendEvent(BMP280_I2C_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Failed to set I2C slave address 0x%02X", BMP280_I2C_ADDR);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

/*
** Close I2C interface
*/
int32 BMP280_CloseI2C(void)
{
    if (BMP280_AppData.I2C_fd >= 0)
    {
        close(BMP280_AppData.I2C_fd);
        BMP280_AppData.I2C_fd = -1;
    }
    return CFE_SUCCESS;
}

/*
** Write to a BMP280 register
*/
int32 BMP280_WriteRegister(uint8 reg, uint8 value)
{
    uint8 buffer[2];
    buffer[0] = reg;
    buffer[1] = value;

    if (write(BMP280_AppData.I2C_fd, buffer, 2) != 2)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

/*
** Read from a BMP280 register
*/
int32 BMP280_ReadRegister(uint8 reg, uint8 *value)
{
    if (write(BMP280_AppData.I2C_fd, &reg, 1) != 1)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (read(BMP280_AppData.I2C_fd, value, 1) != 1)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

/*
** Read multiple bytes from BMP280 registers
*/
int32 BMP280_ReadRegisters(uint8 reg, uint8 *buffer, uint8 length)
{
    if (write(BMP280_AppData.I2C_fd, &reg, 1) != 1)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (read(BMP280_AppData.I2C_fd, buffer, length) != length)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

/*
** Initialize BMP280 sensor
*/
int32 BMP280_InitSensor(void)
{
    int32 status;
    uint8 chip_id;

    /*
    ** Read and verify chip ID
    */
    status = BMP280_ReadRegister(BMP280_REG_CHIPID, &chip_id);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(BMP280_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Failed to read chip ID");
        return status;
    }

    if (chip_id != BMP280_CHIP_ID)
    {
        CFE_EVS_SendEvent(BMP280_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Invalid chip ID 0x%02X (expected 0x%02X)", chip_id, BMP280_CHIP_ID);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /*
    ** Read calibration data
    */
    status = BMP280_ReadCalibration();
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    /*
    ** Configure sensor
    ** osrs_t = 1 (oversampling x1 for temperature)
    ** osrs_p = 1 (oversampling x1 for pressure)
    ** mode = 11 (normal mode)
    ** Control register = 0x27
    */
    status = BMP280_WriteRegister(BMP280_REG_CONTROL, 0x27);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(BMP280_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Failed to write control register");
        return status;
    }

    /*
    ** Configure config register
    ** t_sb = 5 (1000ms standby time)
    ** filter = 0 (filter off)
    ** Config register = 0xA0
    */
    status = BMP280_WriteRegister(BMP280_REG_CONFIG, 0xA0);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(BMP280_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Failed to write config register");
        return status;
    }

    CFE_EVS_SendEvent(BMP280_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BMP280: Sensor initialized successfully (Chip ID: 0x%02X)", chip_id);

    return CFE_SUCCESS;
}

/*
** Read calibration data from BMP280
*/
int32 BMP280_ReadCalibration(void)
{
    uint8 calib[24];
    int32 status;

    /*
    ** Read all calibration registers at once
    */
    status = BMP280_ReadRegisters(BMP280_REG_DIG_T1, calib, 24);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(BMP280_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BMP280: Failed to read calibration data");
        return status;
    }

    /*
    ** Parse calibration data
    */
    BMP280_AppData.CalibData.dig_T1 = (calib[1] << 8) | calib[0];
    BMP280_AppData.CalibData.dig_T2 = (calib[3] << 8) | calib[2];
    BMP280_AppData.CalibData.dig_T3 = (calib[5] << 8) | calib[4];
    BMP280_AppData.CalibData.dig_P1 = (calib[7] << 8) | calib[6];
    BMP280_AppData.CalibData.dig_P2 = (calib[9] << 8) | calib[8];
    BMP280_AppData.CalibData.dig_P3 = (calib[11] << 8) | calib[10];
    BMP280_AppData.CalibData.dig_P4 = (calib[13] << 8) | calib[12];
    BMP280_AppData.CalibData.dig_P5 = (calib[15] << 8) | calib[14];
    BMP280_AppData.CalibData.dig_P6 = (calib[17] << 8) | calib[16];
    BMP280_AppData.CalibData.dig_P7 = (calib[19] << 8) | calib[18];
    BMP280_AppData.CalibData.dig_P8 = (calib[21] << 8) | calib[20];
    BMP280_AppData.CalibData.dig_P9 = (calib[23] << 8) | calib[22];

    return CFE_SUCCESS;
}

/*
** Read raw temperature and pressure data
*/
int32 BMP280_ReadRawData(int32 *adc_T, int32 *adc_P)
{
    uint8 data[6];
    int32 status;

    /*
    ** Read pressure and temperature data (6 bytes total)
    */
    status = BMP280_ReadRegisters(BMP280_REG_PRESS_MSB, data, 6);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    /*
    ** Convert to 20-bit values
    */
    *adc_P = ((int32)data[0] << 12) | ((int32)data[1] << 4) | ((int32)data[2] >> 4);
    *adc_T = ((int32)data[3] << 12) | ((int32)data[4] << 4) | ((int32)data[5] >> 4);

    return CFE_SUCCESS;
}

/*
** Compensate temperature reading (returns degrees Celsius)
*/
float BMP280_CompensateTemperature(int32 adc_T)
{
    int32 var1, var2;
    float T;

    var1 = ((((adc_T >> 3) - ((int32)BMP280_AppData.CalibData.dig_T1 << 1))) *
            ((int32)BMP280_AppData.CalibData.dig_T2)) >>
           11;
    var2 = (((((adc_T >> 4) - ((int32)BMP280_AppData.CalibData.dig_T1)) *
              ((adc_T >> 4) - ((int32)BMP280_AppData.CalibData.dig_T1))) >>
             12) *
            ((int32)BMP280_AppData.CalibData.dig_T3)) >>
           14;

    BMP280_AppData.CalibData.t_fine = var1 + var2;
    T = (BMP280_AppData.CalibData.t_fine * 5 + 128) >> 8;

    return T / 100.0;
}

/*
** Compensate pressure reading (returns Pascals)
*/
float BMP280_CompensatePressure(int32 adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)BMP280_AppData.CalibData.t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)BMP280_AppData.CalibData.dig_P6;
    var2 = var2 + ((var1 * (int64_t)BMP280_AppData.CalibData.dig_P5) << 17);
    var2 = var2 + (((int64_t)BMP280_AppData.CalibData.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)BMP280_AppData.CalibData.dig_P3) >> 8) +
           ((var1 * (int64_t)BMP280_AppData.CalibData.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)BMP280_AppData.CalibData.dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0;  /* Avoid division by zero */
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)BMP280_AppData.CalibData.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)BMP280_AppData.CalibData.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)BMP280_AppData.CalibData.dig_P7) << 4);

    return (float)p / 256.0;
}
