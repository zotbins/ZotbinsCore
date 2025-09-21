## Version 0.1

The current version of newzotbinscore can:

- Detect trash items
- Measure trash weight and bin fullness
- Efficiently manage processing resources by suspending sensor tasks until a piece of trash is detected by the breakbeam
- Connect to the Zotbins AWS IoT MQTT broker
- Serialize and publish data to the MQTT broker as a JSON string

Action items:

- Classes to protect variables
- MCP23017 GPIO extender firmware
- OV2460 external camera module without the ESP-CAM board
- Replace ultrasonic sensor with time of flight sensor
- OTA updates
- Servo motor and timing
- clang-format, automatic build (GitHub Actions)
- Get ESP32 debug board working again
- Change pin layout (ex. Weight sensor uses GPIO 2, however this is a strapping pin which changes the boot mode of the device)
- Bluetooth to interface with bin users
- LCD screen, speaker, and buttons to provide real-time waste info on the bin