/**
 * Author:    Mal Hubert
 * Created:   19.02.2024
 * 
 *   This includes software code developed by third parties, including software code subject to the GNU GPL (General Public license)
 *   Based on partial implementation for the TI80404 https://github.com/sphCow/DAC81404_lib
 *   
 *   Used to find multiple DAC ICs on SPI Bus by using CS pin
 *   
**/

#include <Arduino.h>
#include "dac81416.h"

// DAC Types
#define DACID_81416 0x29C
#define DACID_71416 0x28C
#define DACID_61416 0x24C
#define DACNAME_81416 "Texas Instruments 81416"
#define DACNAME_71416 "Texas Instruments 71416"
#define DACNAME_61416 "Texas Instruments 61416"

// DAC scan results
typedef struct {    
    int CS;
    bool ALIVE;
    int DEVICEID;
    int VERSIONID;
  } DAC;

// Pin definitions
#define DAC_RST 4

// Put CS PINs in here
int DAC_CS_ARRAY[4] = {10, 4, 5, 6};
DAC DAC_ARRAY[4];

// Looks up the DAC
String DACLookup(int deviceid)
{
      switch (deviceid) {
        case DACID_81416:
            return DACNAME_81416;            
        case DACID_71416:
            return DACNAME_71416;
        case DACID_61416:
            return DACNAME_61416;
        default:
            char result[20];
            snprintf(result, sizeof(result), "ID %d Not Found", deviceid);
            return result;
    }
}

// Setup
void setup() {

  Serial.begin(115200);

  // Loop over DAC_CS_ARRAY
  // Initialise DAC
  for(int i=0; i<sizeof(DAC_CS_ARRAY) / sizeof(DAC_CS_ARRAY[0]); i++)
  {
    // Setup the DAC on CS pin from array
    DAC81416 dac(DAC_CS_ARRAY[i], DAC_RST, -1, &SPI, 30000000);

    // Init the DAC
    uint16_t DAC_INIT = dac.init(CRC_DISABLE, DAC81416::U_5);

    // Start populating the result
    DAC_ARRAY[i].CS = DAC_CS_ARRAY[i];

    // Populate the alive status
    DAC_ARRAY[i].ALIVE = dac.is_alive();

    // Check the DAC is alive
    if (DAC_ARRAY[i].ALIVE)
    {
      DAC_ARRAY[i].DEVICEID = dac.get_deviceid();
      DAC_ARRAY[i].VERSIONID = dac.get_versionid();
    }
    else
    {
      DAC_ARRAY[i].DEVICEID = -1;
      DAC_ARRAY[i].VERSIONID = -1;
    }
  } 

  int found_count = 0;

  // Now acess the results
  for(int i=0; i<sizeof(DAC_ARRAY) / sizeof(DAC_ARRAY[0]); i++)
  {
    if (DAC_ARRAY[i].ALIVE)
    {
      found_count++;
      Serial.print("Found DAC on CS PIN: ");
      Serial.print(DAC_ARRAY[i].CS);
      Serial.print(" with DEVICEID ");
      Serial.print(DAC_ARRAY[i].DEVICEID,HEX);
      Serial.print(" (");
      Serial.print(DACLookup(DAC_ARRAY[i].DEVICEID));
      Serial.println(")");
    }
  }

  // Show how many were counted
  Serial.print("Found ");
  Serial.print(found_count);
  Serial.println(" DAC device(s)");
  
} //SETUP

//Not Used
void loop()
{
}
