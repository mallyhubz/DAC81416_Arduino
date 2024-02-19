# DAC81416_Arduino
A basic BETA library for TI's DAC81416 precision DAC and the associated DAC81416EVM reference board

**See `DAC81416.ino` for an working example.**

**See `DACx1416_Scan.ino` for an example looking for multiple DACs.**

**Compile with Arduino IDE or PlatformIO.**

**Platforms:**
I tested the example(s) with Elegoo (Arduino-like) Uno R3. The code should work for other platforms as well. 
Please report if something is not working.

**Dev Setup:**
![alt text](https://github.com/mallyhubz/DAC81416_Arduino/blob/main/dev-setup.jpg?raw=true)

**Limitations:**
1. BETA versions - some bugs may exist. Please report if you encounter something. 
2. LDAC pin is not handled by the library.

**TODO:**
1. Code cleanup
2. Soft and Hard Toggles / Differential + Offsets / Streaming / Broadcast
3. More examples
- Hardware SPI
- Chained
5. Make generic for all 3 DAC types (probably have to rename the repo)