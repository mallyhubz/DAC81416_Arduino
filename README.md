# DAC81416_Arduino
A basic BETA library for TI's DAC81416 precision DAC and the associated DAC81416EVM reference board

**See `main.cpp` for an working example. Compile with Arduino IDE or PlatformIO.**

**Platforms:**
I tested the Exmaple(s) with Elegoo (Arduino-like) Uno R3. The code should work for other platforms as well. 
Please report if something is not working.

**Dev Setup:**
![alt text](https://github.com/mallyhubz/DAC81416_Arduino/blob/main/dev-setup.jpg?raw=true)

**Limitations:**
1. BETA versions - some bugs may exist. Please report if you encounter something. 
2. LDAC pin is not handled by the library.

**TODO:**
1. Code cleanup
2. LDAC SYNC Trigger / Soft and Hard Toggles / Differential + Offsets / Streaming / Broadcast
3. More examples
- Hardware SPI
- Multiple CS
-- Scan for DACS function
- Chained
4. Arduino Uno Wiring Image for EVM board