# Changelog

All notable changes to the BMP280 cFS Application will be documented in this file.

## [1.0.0] - 2025-02-12

### Added
- Initial release of BMP280 sensor application
- I2C communication with BMP280 sensor on STM32MP257F-EV1
- Temperature reading in degrees Celsius
- Pressure reading in Pascals
- Full cFS integration with command and telemetry
- Sensor calibration data handling
- Error detection and reporting
- Performance monitoring via cFS perf logs
- Comprehensive documentation suite:
  - README.md - Main documentation
  - INSTALL.md - Installation guide
  - WIRING_DIAGRAM.md - Hardware connections
  - QUICKSTART.md - Quick start guide
  - SCHEDULER_INTEGRATION.md - Scheduler integration
- Utility scripts:
  - test_hardware.sh - Hardware validation
  - build_bmp280.sh - Build automation
- Three command types:
  - NOOP (command code 0)
  - RESET_COUNTERS (command code 1)
  - READ_SENSOR (command code 2)
- Two telemetry packets:
  - Housekeeping telemetry (MID 0x0884)
  - Sensor data telemetry (MID 0x0885)
- Event messages for all major operations
- Configurable I2C bus and sensor address
- Support for both 0x76 and 0x77 I2C addresses

### Technical Details
- Language: C
- Target Platform: STM32MP257F-EV1 (ARM Cortex-A35)
- Operating System: OpenSTLinux (Yocto-based)
- Toolchain: aarch64-ostl-linux-gcc 13.3.0
- I2C Implementation: Linux i2c-dev interface
- Sensor Protocol: Bosch BMP280 I2C protocol
- Compensation Algorithms: Official Bosch formulas

### Dependencies
- cFS (Core Flight System)
- Linux I2C driver (/dev/i2c-*)
- i2c-dev kernel module
- Standard C library

### Tested With
- cFS Version: Latest from NASA GitHub
- Board: STM32MP257F-EV1 Evaluation Board
- Sensor: BMP280 (Bosch Sensortec)
- Kernel: OpenSTLinux 5.x

### Known Limitations
- Only supports I2C mode (not SPI)
- Single sensor instance
- Blocking I2C reads (no async)
- No sensor self-test functionality
- No advanced filtering options
- Fixed oversampling configuration

### Future Enhancements
Potential features for future versions:
- [ ] SPI interface support
- [ ] Multiple sensor instances
- [ ] Asynchronous I2C operations
- [ ] Configurable oversampling via commands
- [ ] Advanced IIR filter control
- [ ] Sensor self-test command
- [ ] Altitude calculation from pressure
- [ ] Data logging to file
- [ ] BME280 support (with humidity)
- [ ] Temperature/pressure thresholds and alerts

## Version History

### v1.0.0 (February 12, 2025)
- Initial public release
- Fully functional sensor reading
- Complete documentation

---

## Versioning

This project follows [Semantic Versioning](https://semver.org/):
- MAJOR version: Incompatible API changes
- MINOR version: Added functionality (backwards-compatible)
- PATCH version: Bug fixes (backwards-compatible)
- MISSION_REV: Mission-specific revision number

Format: MAJOR.MINOR.PATCH.MISSION_REV

---

## How to Report Issues

If you encounter problems:
1. Check hardware connections (WIRING_DIAGRAM.md)
2. Run test_hardware.sh script
3. Review cFS event log
4. Check I2C with i2cdetect
5. Verify sensor with i2cget

For code issues, please provide:
- cFS version
- Board and OS version
- Complete error messages
- Output of test_hardware.sh
- Steps to reproduce

---

## Contributing

Contributions are welcome! Please:
1. Test thoroughly on actual hardware
2. Update documentation
3. Follow existing code style
4. Add appropriate comments
5. Update this CHANGELOG
