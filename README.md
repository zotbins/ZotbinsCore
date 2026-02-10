## Version 0.1.1 (2/3/2026)

The current version of newzotbinscore can:

- Detect and count trash items, measure trash weight and bin fullness
    - Managed by a GPIO expander to save GPIO pins for additional peripherals
- Efficiently manage processing resources by suspending sensor tasks until a piece of trash is detected by the breakbeam
    - Manage tasks with events instead of sequentially/linearly
- Connect to the Zotbins AWS IoT MQTT broker
- Serialize and publish data to the MQTT broker as a JSON string
- .clang-format provides settings for maintaining code consistency without making any breaking changes
- Automatic build (GitHub Actions)

Action items:

- Classes to promote modularity for additional peripherals
- Write HCSR04 for interrupt capture on GPIO expander
    - Global interrupt handler that is associated with the GPIO manager component
    - Interrupt control flow to handle and identify the various peripherals requiring interrupts
    - Configure interrupt to any edge
- Weight sensor (hx711) verification
- Working OV2640/OV5640
- Interrupt debounce
- Replace ultrasonic sensor with time of flight sensor
- Finalize ervo motor and timing
- Bluetooth to interface with bin users
- LCD screen, speaker, and buttons to provide real-time waste info on the bin
- Implement peripheral_queue, which will manage a queue of messages to publish in case the ESP drops the connection
    - Offline bin has been implemented
- Peripheral device queue, queue measurements in case additional interrupts are detected while the regular interrupt routine is running (LOW PRIORITY)
- Bluetooth
- Integrate OTA
- Remote messsaging web interface
