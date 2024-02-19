// Includes

#include <stdint.h>
#include "Arduino.h"
#include "SPI.h"


// Registers	Table 8-7
#define  R_NOP        0x00
#define  R_DEVICEID   0x01
#define  R_STATUS     0x02
#define  R_SPICONFIG  0x03
#define  R_GENCONFIG  0x04
#define  R_BRDCONFIG  0x05
#define  R_SYNCCONFIG 0x06
#define  R_TOGCONFIG0 0x07
#define  R_TOGCONFIG1 0x08
#define  R_DACPWDWN   0x09
#define  R_DACRANGE0  0x0A
#define  R_DACRANGE1  0x0B
#define  R_DACRANGE2  0x0C
#define  R_DACRANGE3  0x0D
#define  R_TRIGGER    0x0E
#define  R_BRDCAST    0x0F
#define  R_DAC0       0x10
#define  R_DAC1       0x11
#define  R_DAC2       0x12
#define  R_DAC3       0x13
#define  R_DAC4       0x14
#define  R_DAC5       0x15
#define  R_DAC6       0x16
#define  R_DAC7       0x17
#define  R_DAC8       0x18
#define  R_DAC9       0x19
#define  R_DAC10      0x1A
#define  R_DAC11      0x1B
#define  R_DAC12      0x1C
#define  R_DAC13      0x1D
#define  R_DAC14      0x1E
#define  R_DAC15      0x1F

// SPICONFIG 	Table 8-12
#define TEMPALM_EN(x) (x << 11)
#define DACBUSY_EN(x) (x << 10)
#define CRCALM_EN(x)  (x << 9)
#define SFTTOG_EN(x)  (x << 6)
#define DEV_PWDWN(x)  (x << 5)
#define CRC_EN(x)     (x << 4)
#define STR_EN(x)     (x << 3)
#define SDO_EN(x)     (x << 2)
#define FSDO(x)       (x << 1)

// DAC READ MASK
#define RREG 0xC0

// NOP MACRO	62.5ns on 16MHz
#define NOP __asm__("nop\n\t")


class DAC81416 {   
  
    private:
        SPIClass *_spi;
        SPISettings _spi_settings;
        
        // pins
        int _cs_pin;
        int _rst_pin;
        int _ldac_pin;

        inline void cs_on();
        inline void cs_off();
                
        // Might not need NOP, just calling the SPI function is probably delay enough
        inline void tcsh_delay() {
            //delayMicroseconds(1);
			      NOP;
        }

        // SPI functions
        void write_reg(uint8_t reg, uint16_t wdata);
        uint16_t read_reg(uint8_t reg);

        // DACRANGE is a write only register
        // Have to keep track of individual channel ranges manually
        uint16_t KNOWN_DACRANGE[4] = {0,0,0,0};

    public:
    
        // Output Voltage ENUM
        enum ChannelRange {
				   U_5  = 0b0000,   // Unipolar  0V to 5V
                   U_10 = 0b0001,   // Unipolar  0V to 10V
                   U_20 = 0b0010,   // Unipolar  0V to 20V
                   U_40 = 0b0100,   // Unipolar  0V to 40V                   
                   B_5  = 0b1001,   // Bipolar   -5V to +5V                   
                   B_10 = 0b1010,   // Bipolar   -10V to +10V
                   B_20 = 0b1100,   // Bipolar   -20V to +20V
				   B_2V5 = 0b1110}; // Bipolar   -2.5V to +2.5V

		// 2 Types of temporal operation
		// SYNC means that channel will use LDAC LOW to trigger
		// Call LDAC pin LOW with sync()
        enum SyncMode {ASYNC, SYNC};
		
		// Pulls LDAC Low to Sync for SYNC SyncMode pins
		void sync ();

        // LDAC Sync Config
        void set_ch_LDAC_enabled(int ch, bool state);

        // LDAC Sync Config
        bool get_ch_LDAC_enabled(int ch);

        // Default CONFIG
		uint16_t SPICONFIG = TEMPALM_EN(1) | DACBUSY_EN(0) | CRCALM_EN(1) | (0 << 8) | (1 << 7) | SFTTOG_EN(0) | DEV_PWDWN(0) | CRC_EN(0) | STR_EN(0) | SDO_EN(1) | FSDO(1) | 0 << 1;

        // DAC Constructor
        DAC81416(int cspin, int rstpin = -1, int ldacpin = -1,
                 SPIClass *spi = &SPI, uint32_t spi_clock_hz=8000000);

        // Init function to setup the DAC
        int init(ChannelRange default_channelrange);

        // Set DAC channel powerdown
        void set_ch_enabled(int ch, bool state); // true/false = power ON/OFF

        // Get DAC channel power state
        bool get_ch_enabled(int ch);

        // Set Enable Internal 2.5V Reference
        void set_int_reference(bool state);

        // Get Enable Internal 2.5V Reference
        int get_int_reference();
                
        // Set channel range
        void set_range(int ch, ChannelRange range);

        // Get channel range
        int get_range(int ch);

        // Write 16-bit output value
        void set_out(int ch, uint16_t val);

        // Set Sync 
        void set_sync(int ch, SyncMode);

        // Check Alive using R_DEVICEID
    	bool is_alive();

        // Reset
        void reset();

        // Device ID
    	int get_deviceid();

        // Version ID
        int get_versionid();

        // Status
        int get_status();
    	
		// Get temperature
    	float get_temp(int pin, float ref);

};
