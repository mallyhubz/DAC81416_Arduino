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
2. CRC Mode was attempted, but not working

**TODO:**
1. Code and comment cleanup
2. Differential + Offsets / Streaming / CRC
3. Seperate examples
- Hardware SPI
- Chained
5. Make generic for all 3 DAC types (probably have to rename the repo)