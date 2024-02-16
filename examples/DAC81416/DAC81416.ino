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

#define DAC_CS 10
#define DAC_RST 28
#define DAC_LDAC 4
#define DAC_TEMPOUT A0

#define GPIO_TEMPALARM 8
#define DEBUG_FLAG 1

// functions for SYNC output using LDAC and ASYNC output

// function for TEMPALARM

DAC81416 dac(DAC_CS, DAC_RST, DAC_LDAC, &SPI, 30000000);

void setup() {  

  // Initialise DAC
  uint16_t DAC_INIT = dac.init(DAC81416::U_5);
  
  // 2.5V reference
  dac.set_int_reference(false);

  // Set range, U = Unipolar, B = Bipolar; 5, 10, 20, 40, 2V5
  dac.set_range(1, DAC81416::U_10);

  // Enable all DAC OUTPUT channels
  for(int i=0; i<=15; i++)
  {  
      dac.set_ch_enabled(i, true);
  }

  if(DEBUG_FLAG)
  {
    Serial.begin(115200);
    
    Serial.println("init");
    if(dac.is_alive())
    {
      Serial.println("DAC is alive");      
    }

    Serial.print("DAC is device id ");      
    Serial.println(dac.get_deviceid(),HEX);    

    // Check DAC INIT was as expected
    if(dac.SPICONFIG == DAC_INIT)
    {
      Serial.print("DAC81416 init success\n");      
    }
    else
    {
      Serial.print("DAC81416 init fail\n");
      Serial.print("Expected: ");
      Serial.println(dac.SPICONFIG,HEX);
      Serial.print("Got: ");
      Serial.println(DAC_INIT,HEX);
    }

    // Report DAC temperature
    float temperature = dac.get_temp(DAC_TEMPOUT, 5.0);
    Serial.print("DAC Temperature = ");
    Serial.print(temperature);
    Serial.println("*C");

    Serial.print("DAC Internal Ref = ");
    Serial.println(dac.get_int_reference());  
    
    // check channel power status
    for(int i=0; i<=15; i++) 
    {
      Serial.print("ch ");
      Serial.print(i);
      Serial.print(" power_state -> ");
      Serial.println(dac.get_ch_enabled(i));    
    }  
    
    // Read back DAC range
      for(int i=0; i<=15; i++) 
    {
      Serial.print("ch ");
      Serial.print(i);
      Serial.print(" range -> ");
      Serial.println(dac.get_range(i));    
    }
  }
  
  dac.set_sync(1, DAC81416::ASYNC);  
  
}

bool done = true;

void loop() {
  
  dac.set_out(0, 0x0000);
  delay(1000);
  dac.set_out(0, 0x7FFF);
  delay(1000);
  dac.set_out(0, 0xFFFF);
  delay(1000);
  
  /*
  if(millis()>10000 && done) {
    dac.set_ch_enabled(2, false);
    Serial.println("ch 2 shutdown");
    for(int i=0; i<4; i++) Serial.printf("ch %d power -> %d\n", i, dac.get_ch_enabled(i));
    Serial.println();
    done = false;
  } */

}
