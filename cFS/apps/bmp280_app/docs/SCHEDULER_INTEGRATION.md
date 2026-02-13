# Integrating BMP280 App with SCH_LAB

## Overview
This guide shows how to automatically read the BMP280 sensor at regular intervals using the SCH_LAB (Scheduler Lab) application.

## What is SCH_LAB?
SCH_LAB is a simple scheduling application that sends commands at specified frequencies. It's perfect for triggering periodic sensor reads.

## Integration Steps

### Step 1: Locate SCH_LAB Table

The scheduler configuration is in:
```
/home/root/fsw/cFS/apps/sch_lab/fsw/tables/sch_lab_table.c
```

### Step 2: Add BMP280 Read Command

Edit the file:
```bash
nano /home/root/fsw/cFS/apps/sch_lab/fsw/tables/sch_lab_table.c
```

Find the table entries and add a new entry for BMP280:

```c
/*
** Table structure
*/
SCH_LAB_ScheduleTable_t SCH_TBL_Structure = {
    .TickRate = 100,  /* Tick rate in Hz (100 = 10ms per tick) */
    .Config = {
        /* Entry 0: Housekeeping Requests */
        {
            .PacketBuffer = { /* Send HK request to BMP280 */
                {0x18, 0x85}, {0xC0, 0x00}, {0x00, 0x01}, {0x00, 0x00}
            },
            .MessageBuffer = {{0}},
            .TimerId = 1,
            .Frequency = 4,  /* Send every 4 ticks (40ms) */
        },
        
        /* Entry 1: BMP280 Read Sensor Command */
        {
            .PacketBuffer = { /* Send READ_SENSOR command */
                {0x18, 0x84}, {0xC0, 0x00}, {0x00, 0x01}, {0x00, 0x00},
                {0x00, 0x02}  /* Command Code 2 = READ_SENSOR */
            },
            .MessageBuffer = {{0}},
            .TimerId = 2,
            .Frequency = 100,  /* Read every 100 ticks (1 second) */
        },

        /* Add more entries as needed... */
        /* Entry 2-19: (existing entries) */
        
        /* End marker */
        {
            .PacketBuffer = {{0}},
            .MessageBuffer = {{0}},
            .TimerId = 0,
            .Frequency = 0,
        }
    }
};
```

### Step 3: Understanding the Configuration

#### PacketBuffer Format
The packet buffer contains the command message:
- Bytes 0-1: Message ID (0x1884 = BMP280_APP_CMD_MID)
- Bytes 2-3: Function Code and Checksum
- Bytes 4-5: Message Length
- Bytes 6-7: Reserved
- Byte 8: Command Code (0x02 = READ_SENSOR)

#### Frequency Values
- `Frequency = 100`: Every 100 ticks = 1 second (at 100 Hz tick rate)
- `Frequency = 50`: Every 50 ticks = 0.5 seconds
- `Frequency = 200`: Every 200 ticks = 2 seconds
- `Frequency = 1000`: Every 1000 ticks = 10 seconds

#### Calculating Frequency
```
Period (seconds) = Frequency / TickRate
Example: 100 ticks / 100 Hz = 1 second
```

### Step 4: Complete Example with Multiple Frequencies

```c
SCH_LAB_ScheduleTable_t SCH_TBL_Structure = {
    .TickRate = 100,  /* 100 Hz = 10ms per tick */
    .Config = {
        /* Fast sampling: 0.5 seconds */
        {
            .PacketBuffer = {
                {0x18, 0x84}, {0xC0, 0x00}, {0x00, 0x01}, {0x00, 0x00},
                {0x00, 0x02}
            },
            .MessageBuffer = {{0}},
            .TimerId = 1,
            .Frequency = 50,  /* 0.5 seconds */
        },
        
        /* Normal sampling: 1 second */
        {
            .PacketBuffer = {
                {0x18, 0x84}, {0xC0, 0x00}, {0x00, 0x01}, {0x00, 0x00},
                {0x00, 0x02}
            },
            .MessageBuffer = {{0}},
            .TimerId = 2,
            .Frequency = 100,  /* 1 second */
        },
        
        /* Slow sampling: 10 seconds */
        {
            .PacketBuffer = {
                {0x18, 0x84}, {0xC0, 0x00}, {0x00, 0x01}, {0x00, 0x00},
                {0x00, 0x02}
            },
            .MessageBuffer = {{0}},
            .TimerId = 3,
            .Frequency = 1000,  /* 10 seconds */
        },

        /* End marker */
        {
            .PacketBuffer = {{0}},
            .MessageBuffer = {{0}},
            .TimerId = 0,
            .Frequency = 0,
        }
    }
};
```

### Step 5: Rebuild cFS

After modifying the table:

```bash
cd /home/root/fsw/cFS
export CC=aarch64-ostl-linux-gcc
export CXX=aarch64-ostl-linux-g++
rm -rf build
make prep && make && make install
```

### Step 6: Verify Operation

Run cFS and check the event log:
```bash
cd build/exe/cpu1/
./core-cpu1 | grep BMP280
```

You should see periodic messages like:
```
BMP280: Temp=23.45 C, Press=101325.00 Pa
BMP280: Temp=23.46 C, Press=101324.50 Pa
BMP280: Temp=23.45 C, Press=101325.25 Pa
```

## Common Configurations

### Power-Saving Configuration (60 second reads)
```c
.Frequency = 6000,  /* 60 seconds at 100 Hz */
```

### High-Resolution Logging (0.1 second reads)
```c
.Frequency = 10,  /* 0.1 seconds at 100 Hz */
```

### Standard Weather Station (5 minute reads)
```c
.Frequency = 30000,  /* 300 seconds (5 minutes) at 100 Hz */
```

## Troubleshooting

### Problem: Commands not being sent

**Check:**
1. SCH_LAB is loaded in startup script
2. TimerId is unique (not used by other entries)
3. Frequency > 0
4. PacketBuffer has correct message ID

### Problem: Too many reads causing CPU load

**Solution:**
- Increase Frequency value (slower reads)
- Example: Change from 100 to 1000 (1 second to 10 seconds)

### Problem: Timestamps not updating

**Cause:** SCH_LAB tick rate misconfigured

**Solution:**
```c
.TickRate = 100,  /* Verify this matches your system */
```

## Advanced: Multiple Read Rates

You can have different read rates for different purposes:

```c
/* Entry 1: Fast reads for real-time monitoring */
{
    .PacketBuffer = { /* READ_SENSOR */ },
    .Frequency = 10,  /* 0.1 seconds */
    /* Used for: Real-time display */
},

/* Entry 2: Slow reads for data logging */
{
    .PacketBuffer = { /* READ_SENSOR */ },
    .Frequency = 6000,  /* 60 seconds */
    /* Used for: Long-term trending */
},
```

**Note:** This will generate two telemetry packets per cycle. Make sure your telemetry system can handle the data rate.

## Integration with TO_LAB

To route the telemetry, ensure TO_LAB subscribes to BMP280 sensor telemetry:

Edit `/home/root/fsw/cFS/apps/to_lab/fsw/tables/to_lab_sub.c`:

```c
{
    {CFE_SB_MSGID_WRAP_VALUE(BMP280_APP_HK_TLM_MID)},     /* 0x0884 */
    {CFE_SB_MSGID_WRAP_VALUE(BMP280_APP_SENSOR_TLM_MID)}, /* 0x0885 */
    /* ... other subscriptions ... */
}
```

## Performance Considerations

| Read Rate | CPU Usage | Data Rate | Use Case |
|-----------|-----------|-----------|----------|
| 10 Hz (0.1s) | ~2% | High | Real-time control |
| 1 Hz (1s) | <1% | Medium | Monitoring |
| 0.1 Hz (10s) | <0.1% | Low | Data logging |
| 0.017 Hz (60s) | Negligible | Very Low | Weather station |

Choose based on your application requirements and available CPU/bandwidth.

## Example: Weather Station Configuration

```c
/* Weather station: Read every 5 minutes */
{
    .PacketBuffer = {
        {0x18, 0x84}, {0xC0, 0x00}, {0x00, 0x01}, {0x00, 0x00},
        {0x00, 0x02}
    },
    .MessageBuffer = {{0}},
    .TimerId = 1,
    .Frequency = 30000,  /* 300 seconds = 5 minutes */
},
```

This provides regular atmospheric data without overwhelming the system.

## References

- SCH_LAB Documentation: `/home/root/fsw/cFS/apps/sch_lab/README.md`
- cFS Scheduler Design: https://github.com/nasa/SCH
- Command Packet Format: cFS Application Developers Guide
