#include "dac81416.h"

// DAC constructor 
DAC81416::DAC81416(int cspin, int rstpin, int ldacpin, SPIClass *spi, uint32_t spi_clock_hz) {
    _cs_pin = cspin;
    _spi = spi;

    // RESET pin setup
    if(rstpin > -1) _rst_pin = rstpin;
    
    // LDAC pin setup
    if(ldacpin > -1) _ldac_pin = ldacpin;
    
    _spi_settings = SPISettings(spi_clock_hz, MSBFIRST, SPI_MODE0);

    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_rst_pin, HIGH);

    _spi->begin();
}

// CRC calculator
inline uint8_t DAC81416::calculateCRC(uint8_t* data, uint8_t length) {
    uint8_t crc = 0;

    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x8D;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// Chip Select
inline void DAC81416::cs_on() {
    digitalWrite(_cs_pin, LOW);
}

inline void DAC81416::cs_off() {
    digitalWrite(_cs_pin, HIGH);
}

/*

Table 6-1

Active low synchronization signal. When the LDAC pin is low, the DAC outputs of those channels
configured in synchronous mode are updated simultaneously. Connect to VIO if unused.

Table 6-8

LDAC low time   VIO 1.7V to 2.7V	40ns
LDAC low time   VIO 2.7V to 5.5V	20ns

*/

void DAC81416::sync()
{
	digitalWrite(_ldac_pin, LOW);
	NOP;NOP;
	digitalWrite(_ldac_pin, HIGH);
}

/*
  8.6.2  DEVICEID Register
 
	Device ID (MS 14bits)
	Device ID (MS 14bits)
	DAC81416: 29Ch
	DAC71416: 28Ch
	DAC61416: 24Ch
*/

int DAC81416::get_deviceid()
{
	uint16_t deviceID = read_reg(R_DEVICEID);		
	
	// DAC81416 will return 29C if it is alive
	return deviceID >> 0x02;
}

int DAC81416::get_versionid()
{
  uint16_t deviceV = read_reg(R_DEVICEID);   
  
  // DAC81416 will return 2 bits version ID (0 on DAC81416EVM)
  return deviceV &= 0x02;
}

bool DAC81416::is_alive()
{
	uint16_t deviceID = read_reg(R_DEVICEID);	

  // DAC (of some type) is alive
  // Use get_device_id() to check exactly which type
	if (deviceID == 0xFFFF || deviceID == 0x0000)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int DAC81416::init(bool CRC, ChannelRange default_channelrange) {
        
    if(_rst_pin!=-1) {
        pinMode(_rst_pin, OUTPUT);
        digitalWrite(_rst_pin, LOW);
        delay(1); 
        digitalWrite(_rst_pin, HIGH); 
        delay(1);
    }

    // Enable SDO
    write_reg(R_SPICONFIG, 0x0004);
	  delay(1);
   
    // Set SPICONFIG
	if (CRC == 0)
	{		
		write_reg(R_SPICONFIG, SPICONFIG);
	}
	else
	{
		// Don't use CRC Mode, not implemented
		//write_reg(R_SPICONFIG, CRC_SPICONFIG);
	}
	delay(1);

    // Set the default channel RANGES
  	for(int i=0; i<=15; i++)
  	{
  		set_range(i, default_channelrange);	
  	}

    // Used to check if it was set correctly
  	return (read_reg(R_SPICONFIG));
}


// SPAM THE NON-CRC SPICONFIG UNTIL IT WORKS - NEED TO FIGURE OUT CRC ISSUES
void DAC81416::fix() {

	//TRY EVERY CRC, FOR SOME REASON WITH THE SAME DATA IT WANTS A DIFFERENT CRC EACH TIME I TRY IT
	for(int i=0; i<=255; i++) {
    _spi->beginTransaction(_spi_settings);
    cs_on();
    NOP;
    _spi->transfer(0x03); // SPICONFIG Register
    _spi->transfer(0x08); // SPICONFIG Default
    _spi->transfer(0x86); // SPICONFIG Default
	_spi->transfer(i); 
    tcsh_delay();
    cs_off();
    _spi->endTransaction();
	delay(100);
	Serial.print("Trying... ");
	Serial.println(i,HEX);
	Serial.print("Result: ");
	Serial.println(get_deviceid(),HEX);
	
	if(get_deviceid() == 0x29C){ break; };
		//STOP ONCE IT'S FIXED (81416)
	}
}

void DAC81416::write_reg(uint8_t reg, uint16_t wdata) {
    uint8_t lsb = ((uint16_t)wdata >> 0) & 0xFF;
    uint8_t msb = ((uint16_t)wdata >> 8) & 0xFF;

    _spi->beginTransaction(_spi_settings);
    cs_on();
    NOP;
    _spi->transfer(reg);
    _spi->transfer(msb);
    _spi->transfer(lsb);
    tcsh_delay();
    cs_off();
    _spi->endTransaction();
}


uint16_t DAC81416::read_reg(uint8_t reg) {
    uint8_t buf[3]; // 15-0

    _spi->beginTransaction(_spi_settings);
    cs_on();
    _spi->transfer( (RREG | reg) );
    _spi->transfer(0x00);
    _spi->transfer(0x00);
    tcsh_delay();
    cs_off();

    cs_on();
    buf[0] = _spi->transfer(0x00);
    buf[1] = _spi->transfer(0x00);
    buf[2] = _spi->transfer(0x00);
    tcsh_delay();
    cs_off();
    _spi->endTransaction();

    // check buf[0]

    uint16_t res = ((buf[1] << 8) | buf[2]);
    return res;
}

// TESTING, VERY MUCH NOT WORKING
uint16_t DAC81416::read_reg_crc(uint8_t reg) {
    uint8_t buf[4]; // 31-0	
	
	byte data[] = {(0x80 | reg), 0x00, 0x00};
	byte dataSize = sizeof(data) / sizeof(data[0]);
	
	uint8_t CRC = calculateCRC(data, dataSize);
	
    _spi->beginTransaction(_spi_settings);
    cs_on();
    _spi->transfer( (0x80 | reg) );
    _spi->transfer(0x00); // Don't care values
    _spi->transfer(0x00); // Don't care values
	_spi->transfer(CRC);  // Calculated CRC
    tcsh_delay();
    cs_off();

    cs_on();
    buf[0] = _spi->transfer(0x00);
    buf[1] = _spi->transfer(0x00);
    buf[2] = _spi->transfer(0x00);
	buf[3] = _spi->transfer(0x00);
    tcsh_delay();
    cs_off();
    _spi->endTransaction();

    // buf[0] will be the CRC
	// buf[1] and buf[2] are the data
	// buf[3] is address, CRC bit and RW bit	

	Serial.println(buf[0],HEX);
	Serial.println(buf[1],HEX);
	Serial.println(buf[2],HEX);
	Serial.println(buf[3],HEX);

    uint16_t res = ((buf[2] << 8) | buf[1]);
    return res;
}

//************** Set/Get Channel Status **************//
// Corrected for channels 0 to 15
void DAC81416::set_ch_enabled(int ch, bool state) { // true/false = power ON/OFF
    uint16_t res = read_reg(R_DACPWDWN);
    
    // if state==true, power up the channel
    if(state) write_reg(R_DACPWDWN, res &= ~(1 << ch) );
    else write_reg(R_DACPWDWN, res |= (1 << ch) );

}

bool DAC81416::get_ch_enabled(int ch) {
	
    uint16_t res = read_reg(R_DACPWDWN);
    return !(bool(((res >> ch) & 1)));
}

//************** Set/Get Broadcast Enable Status **************//
/*
8.6.6

When set to 1 the corresponding DAC is set to update its output to
the value set in the BRDCAST register. All DAC channels must be
configured in single-ended mode for broadcast operation. If one or
more outputs are configured in differential mode the broadcast mode
is ignored.

When cleared to 0 the corresponding DAC output remains unaffected
by a BRDCAST command.

*/

//************** Set/Get Broadcast Enable Status **************//
void DAC81416::set_ch_broadcast(int ch, bool state) { // true/false = power ON/OFF
    uint16_t res = read_reg(R_BRDCONFIG);
    
    // if state==true, power up the channel
    if(state) write_reg(R_BRDCONFIG, res &= ~(1 << ch) );
    else write_reg(R_BRDCONFIG, res |= (1 << ch) );

}

bool DAC81416::get_ch_broadcast(int ch) {
	
    uint16_t res = read_reg(R_BRDCONFIG);
    return !(bool(((res >> ch) & 1)));
}

//************** Set/Get LDAC Enable Status **************//
// Corrected for channels 0 to 15
void DAC81416::set_ch_LDAC_enabled(int ch, bool state) { // true/false = power ON/OFF
    uint16_t res = read_reg(R_SYNCCONFIG);
    
    // if state==true, power up the channel
    if(state) write_reg(R_SYNCCONFIG, res &= ~(1 << ch) );
    else write_reg(R_SYNCCONFIG, res |= (1 << ch) );

}

// Corrected for channels 0 to 15
bool DAC81416::get_ch_LDAC_enabled(int ch) {
  
    uint16_t res = read_reg(R_SYNCCONFIG);
    return !(bool(((res >> ch) & 1)));
}

//************** Set internal reference **************//
void DAC81416::set_int_reference(bool state) {
    if(state) write_reg(R_GENCONFIG, (0 << 14) ); // Turn on 2.5V reference
    else write_reg(R_GENCONFIG, (1 << 14) ); // Shutdown 2.5V reference
}

//************** Get internal reference **************//
int DAC81416::get_int_reference() {

    uint16_t res = read_reg(R_GENCONFIG);
    return (res >> 14) & 0x01;
}

//**************** Set Range of a channel ***************//
void DAC81416::set_range(int ch, ChannelRange range) {

	// Calculate which REGISTER from Channel Number 0 to 15;
	int reg = ch / 4;
	
	// REGISTER address to be stored in here
	int DAC_REGISTER = R_DACRANGE0 + reg;
	
	// From original library - Calculate the bits needed to configure register
    uint16_t mask = (0xffff >> (16-4)) << 4*ch;
    uint16_t write = (KNOWN_DACRANGE[DAC_REGISTER - 0x0A] & ~mask) | (( range << 4*ch )&mask);
	
	// Update saved DACRANGE state
    KNOWN_DACRANGE[DAC_REGISTER - 0x0A] = write;		
			
	// Write to SPI
    write_reg(DAC_REGISTER, write);
}

// Only gets what the MCU has set this BOOT
int DAC81416::get_range(int ch) {
	
	// Calculate which REGISTER from Channel Number 0 to 15;
	int reg = ch / 4;
	
	// REGISTER address to be stored in here
	int DAC_REGISTER = R_DACRANGE0 + reg;
	
	// Select correct known DAC range config from teh array using channel register	
	int dacRangeIndex = DAC_REGISTER - 0x0A;
	
	// Get it
	uint8_t val = (KNOWN_DACRANGE[dacRangeIndex] >> 4*ch) & (0xF);
	
	// Return it
    return val;
}

/*

8.6.13

Writing to the BRDCAST register forces those DAC channels that
have been set to broadcast in the BRDCONFIG register to update its
data register data to the BRDCAST one

*/

//************ Write value to broadcast channels ***********//
void DAC81416::set_out_broadcast(uint16_t val) {
	
	// Register for Broadcast
    write_reg(R_BRDCAST, val);
}


//**************** Write value to a channel ***************//
void DAC81416::set_out(int ch, uint16_t val) {
	
	// Registers are sequential so add channel to first DAC output register address
    write_reg(R_DAC0+ch, val);
}

//************* Set/get sync mode of a channel ************//
/*
Table 8-15

When set to 1 the corresponding DAC output is set to update in
response to an LDAC trigger (synchronous mode).
When cleared to 0 the corresponding DAC output is set to update
immediately (asynchronous mode).

*/
void DAC81416::set_sync(int ch, SyncMode mode) {
	
	// Read R_SYNCCONFIG register
    uint16_t read = read_reg(R_SYNCCONFIG);
	
    //Serial.print("sync read -> "); Serial.println(read, HEX);
	
	// Update bits for my channel ch
    if(mode==SYNC) read |= 1UL << ch;
    else read &= ~(1UL << ch);
	
	// Write the register back
    write_reg(R_SYNCCONFIG, read);
	
    //Serial.print("sync wrote -> "); Serial.println(read, HEX);
}


// Set DAC Channel Toggle Config
void DAC81416::set_ch_togglemode(int ch, ToggleMode mode)
{
	uint8_t reg = ch < 8 ? R_TOGCONFIG1 : R_TOGCONFIG0;
	// Select correct TOG REGISTER from Channel
	uint16_t read = read_reg(reg);
	
	// Select correct TOG REGISTER from Channel and know which bits to update
	/*uint8_t bi = 0;		
	
	if (ch < 8)
	{
		bi = ch * 2;				
	}
	else
	{
		bi = (ch - 8) * 2;		
	}
	*/
	
	uint16_t bi = (ch % 8) * 2;
		
    //Serial.print("togg  prewrite -> "); Serial.println(read, BIN);
		
	// Update the bits
	  switch (mode) {
		case 0: // 00
		  read &= ~(0b11 << bi);
		  break;
		case 1: // 01
		  read |= (0b01 << bi);
		  read &= ~(1 << (bi + 1));
		  break;
		case 2: // 10
		  read |= (0b10 << bi);
		  read &= ~(1 << bi);
		  break;
		case 3: // 11
		  read |= (0b11 << bi);
		  break;
	  }

	//Serial.print("togg postwrite -> "); Serial.println(read, BIN);
	
	// Write the register back
    write_reg(reg, read);
}

// Set DAC Channel Toggle Config
int DAC81416::get_ch_togglemode(int ch)
{
	// Select correct TOG REGISTER from Channel
	uint16_t read = read_reg(ch < 8 ? R_TOGCONFIG1 : R_TOGCONFIG0);
	
	// Bit index of the LSB of the 2 bits you want to read
	uint8_t bi = (ch % 8) * 2;
	
	// Bit mask the bits I want
	uint16_t bitmask = 0b11 << (bi);
	
	// Extract the bits
	uint16_t ex = (read & bitmask) >> (bi);
	return ex;
}


int DAC81416::get_status()
{
    // Read R_STATUS register
    uint16_t read = read_reg(R_STATUS);
    //uint16_t read = 0x01;
    read &= 0x07;
    // Return it as-is
    return(read);
}




//****************** Reset DAC ******************//
/* 

Table 6-1 & 8.3.3.2

Active low reset input. Logic low on this pin causes the device to issue a power-on-reset event.
A device hardware reset event is initiated by a minimum 500 ns logic low on the RESET pin.

*/
void DAC81416::reset()
{  
  digitalWrite(_rst_pin, LOW);
  delay(1);
  digitalWrite(_rst_pin, HIGH);
}




//*********** Trigger reset to Defaults ***********//
/*

8.6.12

When set to the reserved code 1010 resets the device to its default
state.

*/
void DAC81416::defaults()
{
	write_reg(R_TRIGGER, DEVICE_DEFAULTS_CODE);
}




//************* Trigger a Toggle **************//
/*
8.6.12

If soft toggle is enabled set, this bit controls the toggle between
values for those DACs that have been set in toggle mode 2 in the
TOGGCONFIG register. Set to 1 to update to Register B and clear to
0 for Register A.

Enable SOFT TOGGLE by updating SPICONFIG before dac.init() with SFTTOG_EN(1)

e.g SPICONFIG = TEMPALM_EN(1) | DACBUSY_EN(0) | CRCALM_EN(0) | (0 << 8) | (1 << 7) | SFTTOG_EN(1) | DEV_PWDWN(0) | CRC_EN(0) | STR_EN(0) | SDO_EN(1) | FSDO(1) | 0 << 1;

*/
void DAC81416::trigger_toggle(ToggleMode toggle)
{	
	// Cant use the "NOTOGGLE" ToggleMode to select which Toggle to toggle!
	if (toggle != NOTOGGLE)
	{
		write_reg(R_TRIGGER, (1 << toggle + 4));
	}
}




//************* Trigger LDAC **************//
/*

8.6.12
Set this bit to 1 to synchronously load those DACs who have been
set in synchronous mode in the SYNCCONFIG register.

Doesn't check if anything is in the SYNCCONFIG register though

*/
void DAC81416::trigger_ldac()
{
	// Write into LDAC bit posistion
	write_reg(R_TRIGGER, (1 << TRIGGER_LDAC) ); 
}




//************* Trigger Alarm RESET **************//
/*

8.6.12
Set this bit to 1 to clear an alarm event. Not applicable for a DACBUSY alarm event.

*/
void DAC81416::trigger_alarm_reset()
{
	// Write into Alarm Reset bit posistion 
	write_reg(R_TRIGGER, (1 << TRIGGER_ALMRST) );  
}





//******************* Temperature Sensor  ******************//
/* 
8.3.4.1

The DACx1416 includes an analog temperature monitor with an unbuffered output voltage that is inversely
proportional to the device junction temperature. The TEMPOUT pin output voltage has a temperature slope of
–4 mV/°C and a 1.34-V offset as described by Equation.

V_TEMPOUT = (( -4 mV / *C ) * T) + 1.34V

where:
• T is the device junction temperature in °C.
• V_TEMPOUT is the temperature monitor output voltage

*/
float DAC81416::get_temp(int pin, float ref)
{
   int sensorValue = analogRead(pin);
   float voltage = ((sensorValue / 1023.0) * ref);
   float temperature = ((voltage - 1.34) / -0.004);
  
   return temperature;
}


