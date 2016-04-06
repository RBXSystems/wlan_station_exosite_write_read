//*****************************************************************************
// cloud_demo.h
//
// Write and Read data to Exosite Cloud using Tiva Connected Launchpad + CC3100 BP + Sensor Hub BP,
// Connects to AP through WiFi
//
// Maker/Author - Markel T. Robregado *
// Modifications - Richard Carpenter
// Modification Details : Write and read data to Exosite.
// Added readTmp006Data, readBmp180Data and readSht21Data functions
//
// Device Setup: Tiva Connected Launchpad + CC3100 Booster pack + Sensor Hub Booster Pack 
//
//*****************************************************************************

#ifndef CLOUD_DEMO_H_
#define CLOUD_DEMO_H_

void SW1_Pressed(void);
void SW2_Pressed(void);
void Demo_Tick(void);
void cloud_demo(void);

void readTmp006Data(void);
void readBmp180Data(void);
void readSht21Data(void);

void SHT21AppCallback(void *pvCallbackData, uint_fast8_t ui8Status);

#endif /* CLOUD_DEMO_H_ */
