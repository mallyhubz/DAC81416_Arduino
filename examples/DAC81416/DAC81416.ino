/**
 * Author:    Mal Hubert
 * Created:   15.02.2024
 * 
 *   This includes software code developed by third parties, including software code subject to the GNU GPL (General Public license)
 *   Based on partial implementation for the TI80404 https://github.com/sphCow/DAC81404_lib
 *   
**/

/*
Working notes 
 
When the LDAC pin is low, the DAC outputs of those channels
configured in synchronous mode are updated simultaneously. Connect to VIO if unused.
*/

#include <Arduino.h>
#include "dac81416.h"

// Pin definitions
#define DAC_CS 10
#define DAC_RST 4
#define DAC_LDAC 4
#define DAC_TEMPOUT A0

#define GPIO_TEMPALARM 8
#define DEBUG_FLAG 1

// See README for TO DO list

// Declare DAC. For non-chained-mode, use a CS pin per DAC
DAC81416 dac(DAC_CS, DAC_RST, DAC_LDAC, &SPI, 30000000);

void setup() {  

  // Initialise DAC
  uint16_t DAC_INIT = dac.init(DAC81416::U_5);
  
  // 2.5V reference
  dac.set_int_reference(false);

  // Set range, U = Unipolar, B = Bipolar; 5, 10, 20, 40, 2V5
  dac.set_range(1, DAC81416::U_10);

  // Enable all DAC OUTPUT channels as ASYNC
  for(int i=0; i<=15; i++)
  {  
      dac.set_ch_enabled(i, true);
      dac.set_sync(i, DAC81416::ASYNC);
  }

  if(DEBUG_FLAG)
  {
    Serial.begin(115200);
    
    Serial.println("DAC Init...");
    if(dac.is_alive())
    {
      Serial.println("DAC is alive");      
      Serial.print("DAC is device id ");      
      Serial.println(dac.get_deviceid(),HEX);    
      
      // Check DAC INIT was as expected
      if(!dac.SPICONFIG == DAC_INIT)
      {
        Serial.print("DAC81416 SPICONFIG FAILED\n");
        Serial.print("Expected: ");
        Serial.println(dac.SPICONFIG,HEX);
        Serial.print("Received: ");
        Serial.println(DAC_INIT,HEX);      
      }

      // Report DAC temperature
      float temperature = dac.get_temp(DAC_TEMPOUT, 5.0);
      Serial.print("DAC Temperature = ");
      Serial.print(temperature);
      Serial.println("*C");
  
      Serial.print("DAC Internal Ref = ");
      Serial.println(dac.get_int_reference());  
      
      // Report Channel Enable Status
      for(int i=0; i<=15; i++) 
      {
        Serial.print("ch ");
        Serial.print(i);
        Serial.print(" enable_state -> ");
        Serial.println(dac.get_ch_enabled(i));    
      }  
      
      // Report DAC RANGE Config
      for(int i=0; i<=15; i++) 
      {
        Serial.print("ch ");
        Serial.print(i);
        Serial.print(" range -> ");
        Serial.println(dac.get_range(i));    
      }
     }
     else
     {
        Serial.println("DAC is not responding");    
     }
  }
} //SETUP

// A 5 seconds task to check the STATUS register
const unsigned long interval = 5000;
unsigned long previousMillis = 0;
bool taskExecuted = false;

/*
   MAIN LOOP
*/

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval && !taskExecuted) {
    
    // Execute 5 second task    
    int read_status = dac.get_status();
    
    if(DEBUG_FLAG)
    {
        Serial.print("Status REGISTER = ");
        Serial.println(read_status,HEX);
    }

    // Check each STATUS bit
    if (read_status & 0b001) {
        Serial.println("TEMPERATURE ALARM");
    }

    if (read_status & 0b010) {
        Serial.println("DAC BUSY");
    }

    if (read_status & 0b100) {
        Serial.println("CRC ALARM");
    }

    // Set the flag to indicate that the task has been executed
    taskExecuted = false;

    // Update the previousMillis to the current time
    previousMillis = currentMillis;
  }

  /*
  dac.set_out(0, 0x0000);
  delay(1000);
  dac.set_out(0, 0x7FFF);
  delay(1000);
  dac.set_out(0, 0xFFFF);
  delay(1000);
  */

}
