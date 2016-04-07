//*****************************************************************************
// cloud_demo.c
//
// Write and Read data to Exosite Cloud using Tiva Connected Launchpad + CC3100 BP + Sensor Hub BP,
// Connects to AP through WiFi
//
// Maker/Author - Markel T. Robregado *
// Modification Details : Write and read data to Exosite.
// Sends Temperature data from TMP006 sensor to Exosite Cloud
// Sends switch press count to Exosite Cloud
// on-board led on-off, from Exosite Dashboard Switches
//
// Device Setup: Tiva Connected Launchpad + CC3100 Booster pack + Sensor Hub Booster Pack 
//
//*****************************************************************************

#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sl_common.h"
#include "exosite.h"
#include "exosite_pal.h"
#include "cloud_demo.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "drivers/pinout.h"
#include "utils/uartstdio.h"
#include "sensorlib/hw_sht21.h"
#include "sensorlib/i2cm_drv.h"
#include "sensorlib/tmp006.h"
#include "sensorlib/bmp180.h"
#include "sensorlib/sht21.h"
#include "sensorlib/isl29023.h"
#include "sensorlib/hw_isl29023.h"

#define TEMP_ALIAS          "tmp006"
#define BMPT_ALIAS           "bmp180_T"
#define BMPP_ALIAS           "bmp180_P"
#define SHTT_ALIAS           "sht21_T"
#define SHTH_ALIAS           "sht21_H"
#define ISL_ALIAS           "isl29023"
#define LED2_ALIAS          "ledd2"
#define LED3_ALIAS          "ledd3"
#define SW1_ALIAS           "usrsw1"
#define SW2_ALIAS           "usrsw2"

#define TEMP_ALIAS_LENGTH          6
#define BMP_ALIAS_LENGTH           8
#define SHT_ALIAS_LENGTH           7
#define ISL_ALIAS_LENGTH           8
#define LED2_ALIAS_LENGTH          5
#define LED3_ALIAS_LENGTH          5
#define SW1_ALIAS_LENGTH           6
#define SW2_ALIAS_LENGTH           6

// Global instance structure for the TMP006 sensor driver.
extern tTMP006 g_sTMP006Inst;

//*****************************************************************************
//
// Global instance structure for the BMP180 sensor driver.
//
//*****************************************************************************
extern tBMP180 g_sBMP180Inst;

//*****************************************************************************
//
// Global instance structure for the SHT21 sensor driver.
//
//*****************************************************************************
extern tSHT21 g_sSHT21Inst;

//*****************************************************************************
//
// Global instance structure for the ISL29023 sensor driver.
//
//*****************************************************************************
extern tISL29023 g_sISL29023Inst;

extern unsigned long ui32SysClock;
extern unsigned long g_Status;

volatile int sw1_button_on = 0;
volatile int sw2_button_on = 0;

// Global new data flag to alert main that TMP006 data is ready.
extern volatile uint_fast8_t g_vui8DataFlag;
// Global new error flag to store the error condition if encountered.
extern volatile uint_fast8_t g_vui8ErrorFlag;

extern void TMP006AppErrorHandler(char *pcFilename, uint_fast32_t ui32Line);

char post_str[512];
int post_len = 0;

//*****************************************************************************
//
// Constants to hold the floating point version of the thresholds for each
// range setting. Numbers represent an 81% and 19 % threshold levels. This
// creates a +/- 1% hysteresis band between range adjustments.
//
//*****************************************************************************
const float g_fThresholdHigh[4] =
{
    810.0f, 3240.0f, 12960.0f, 64000.0f
};
const float g_fThresholdLow[4] =
{
    0.0f, 760.0f, 3040.0f, 12160.0f
};

/*****************************************************************************
*
* blinky_led1
*
*  \param  times - count times
*
*  \return None
*
*  \brief  control the led7 blinky times
*
*****************************************************************************/
void blinky_led1(int times)
{
	int intern_cnt = times;

	while (intern_cnt > 0)
	{
		LEDWrite(CLP_D1, CLP_D1);
		SysCtlDelay(ui32SysClock / (4 * 3)); // 250 ms
		LEDWrite(CLP_D1, ~CLP_D1);
		SysCtlDelay(ui32SysClock / (4 * 3)); // 250 ms
		intern_cnt--;
	}
}

/*****************************************************************************
*
* SW2_Pressed
*
*  \param  None
*
*  \return None
*
*  \brief  Handle the Switch 2 press event
*
*****************************************************************************/
void SW1_Pressed(void)
{
	sw1_button_on++;
	if(sw1_button_on == 15 )
	{
		sw1_button_on = 0;
	}
	UARTprintf(" SW1 pressed.\r\n");
}

/*****************************************************************************
*
* SW3_Pressed
*
*  \param  None
*
*  \return None
*
*  \brief  Handle the Switch 3 press event
*
*****************************************************************************/
void SW2_Pressed(void)
{
	sw2_button_on++;
	if(sw2_button_on == 15 )
	{
		sw2_button_on = 0;
	}
	UARTprintf(" SW2 pressed.\r\n");
}

/*****************************************************************************
*
* Status_Indicate
*
*  \param  None
*
*  \return None
*
*  \brief  Blinky the LED D7 to indicate the current status
*
*****************************************************************************/
void Status_Indicate(void)
{
  int state = Exosite_StatusCode();

  if (EXO_STATE_R_W_ERROR == state)
  {
    blinky_led1(2);
  }

}

/*****************************************************************************
*
* Cloud_Read
*
*  \param  None
*
*  \return None
*
*  \brief  Reads data from Exosite Cloud, and turn ON/OFF led .. etc. from Cloud
*
*****************************************************************************/
void Cloud_Read(void)
{

	char ledx[10] = "DEADBEEFDE";
	int32_t Read_status = 0;
	int32_t	switch1_data = 0;
	int32_t	switch2_data = 0;
	uint16_t response_length = 0;


	//use exosite_read to read multiple aliases values
	//returns "ledd2=0"
	Read_status = exosite_read("ledd2", ledx, 10, &response_length);

	if (Read_status == 0)
	{
		switch1_data = exoPal_atoi(&ledx[strlen(LED2_ALIAS) + 1]);
		if (switch1_data == 1)
		{
			LEDWrite(CLP_D2, CLP_D2);
		}
		if (switch1_data == 0)
		{
			LEDWrite(CLP_D2, ~CLP_D2);
		}
		//UARTprintf(" Exosite Read:  %s=%d\r\n", LED2_ALIAS, switch1_data);
	}

	Read_status = exosite_read("ledd3", ledx, 10, &response_length);

	if (Read_status == 0)
	{
		switch2_data = exoPal_atoi(&ledx[strlen(LED3_ALIAS) + 1]);
		if (switch2_data == 1)
		{
			LEDWrite(CLP_D3, CLP_D3);
		}
		if (switch2_data == 0)
		{
			LEDWrite(CLP_D3, ~CLP_D3);
		}
		//UARTprintf(" Exosite Read:  %s=%d\r\n", LED3_ALIAS, switch2_data);
	}
}

/*****************************************************************************
*
* Report_Sensors
*
*  \param  None
*
*  \return None
*
*  \brief  Posts the data to Exosite Cloud
*
*****************************************************************************/
void Report_Sensors(void)
{
	post_str[512];
	post_len = 0;

	readTmp006Data();

	readBmp180Data();

	readSht21Data();

	readIsl29023Data();


	//UARTprintf(" Exosite Write: %s\r\n", post_str);

	//Exosite Write: usrsw1=0&usrsw2=0&tmp006=24.93&bmp180_T=23.850&bmp180_P=100266.20&sht21_H=47.764&sht21_T=16.63&isl29023=63.980

	UARTprintf(".");

	exosite_write(post_str, post_len);
}

/*****************************************************************************
*
* cloud_demo
*
*  \param  None
*
*  \return None
*
*  \brief  The Exosite Cloud main application
*
*****************************************************************************/
void cloud_demo(void)
{

	int exo_state = -1;
	unsigned int delay_multiplier = 1;
	unsigned int read_interval = 2;
	unsigned int write_interval = 4;
	unsigned int interval_counter = 0;

	UARTprintf("\r\n\r\n");
	UARTprintf(" Exosite Cloud App Start.\r\n");

	exo_state = EXO_STATUS_OK; //No status code return yet from Exosite

	while (1)
	{
		if (IS_CONNECTED(g_Status))
		{
			if (EXO_STATUS_OK == exo_state)
			{
				if(interval_counter % write_interval == 0)
				{
					Report_Sensors();
				}

				if(interval_counter % read_interval == 0)
				{
					Cloud_Read();
				}
			}

			if (EXO_STATE_R_W_ERROR == exo_state)
            {
				UARTprintf(" Unable to connect to Exosite check CIK\r\n");
            }


			delay_multiplier = 1;

		}
		else
		{
			UARTprintf(" WiFi Disconnected\r\n");
			UARTprintf(" Check connections and restart device . . .\r\n");
			delay_multiplier = 60;
		}

		Status_Indicate();
		SysCtlDelay(delay_multiplier * (ui32SysClock / (2 * 3))); //    * 500 ms

		if(interval_counter == UINT_MAX) //UINT_MAX from limits.h value 65536
		{
			interval_counter = 0;
		}

		interval_counter++;
	}
}

void readTmp006Data(void)
{
	float fAmbient, fObject;
    int_fast32_t i32IntegerPart;
    int_fast32_t i32FractionPart;

	//
	// Put the processor to sleep while we wait for the TMP006 to
	// signal that data is ready.  Also continue to sleep while I2C
	// transactions get the raw data from the TMP006
	//

	while((g_vui8DataFlag == 0) && (g_vui8ErrorFlag == 0))
	{
		//ROM_SysCtlSleep();
	}


	//
	// If an error occurred call the error handler immediately.
	//
	if(g_vui8ErrorFlag)
	{
		TMP006AppErrorHandler(__FILE__, __LINE__);
	}

	//
	// Reset the flag
	//
	g_vui8DataFlag = 0;

	//
	// Get a local copy of the latest data in float format.
	//
	TMP006DataTemperatureGetFloat(&g_sTMP006Inst, &fAmbient, &fObject);

    // Convert the floating point ambient temperature  to an integer part
    // and fraction part for easy printing.
    //
    i32IntegerPart = (int32_t)fAmbient;
    i32FractionPart = (int32_t)(fAmbient * 1000.0f);
    i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
    if(i32FractionPart < 0)
    {
        i32FractionPart *= -1;
    }

	memcpy(&post_str[post_len], SW1_ALIAS, SW1_ALIAS_LENGTH);
	post_len += strlen(SW1_ALIAS);
	post_str[post_len] = '=';
	post_len++;
	sprintf(&post_str[post_len], "%d", (char)sw1_button_on);

	post_len = strlen(post_str);
	post_str[post_len] = '&';
	post_len++;

	memcpy(&post_str[post_len], SW2_ALIAS, SW2_ALIAS_LENGTH);
	post_len += strlen(SW2_ALIAS);
	post_str[post_len] = '=';
	post_len++;
	sprintf(&post_str[post_len], "%d", (char)sw2_button_on);

	post_len = strlen(post_str);
	post_str[post_len] = '&';
	post_len++;

	//last
	memcpy(&post_str[post_len], TEMP_ALIAS, TEMP_ALIAS_LENGTH);
	post_len += strlen(TEMP_ALIAS);
	post_str[post_len] = '=';
	post_len++;
	post_len += snprintf(&post_str[post_len], 10, "%2d.%02d", i32IntegerPart, i32FractionPart);

	post_len = strlen(post_str);
	post_str[post_len] = '&';
	post_len++;

}

void readBmp180Data(void)
{
    float fTemperature, fPressure, fAltitude;
    int32_t i32IntegerPart;
    int32_t i32FractionPart;

    //
    // The reads are started by SysTick Interrupt, we poll here to detect
    // when a read is complete.
    //
    while(g_vui8DataFlag == 0)
    {
        //
        // Wait for the new data set to be available.
        //
    }

    //
    // Reset the data ready flag.
    //
    g_vui8DataFlag = 0;

    //
    // Get a local copy of the latest temperature and pressure data in
    // float format.
    //
    BMP180DataTemperatureGetFloat(&g_sBMP180Inst, &fTemperature);
    BMP180DataPressureGetFloat(&g_sBMP180Inst, &fPressure);

    //
    // Convert the temperature to an integer part and fraction part for
    // easy print.
    //
    i32IntegerPart = (int32_t) fTemperature;
    i32FractionPart =(int32_t) (fTemperature * 1000.0f);
    i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
    if(i32FractionPart < 0)
    {
        i32FractionPart *= -1;
    }

	memcpy(&post_str[post_len], BMPT_ALIAS, BMP_ALIAS_LENGTH);
	post_len += strlen(BMPT_ALIAS);
	post_str[post_len] = '=';
	post_len++;
	post_len += snprintf(&post_str[post_len], 10, "%2d.%02d", i32IntegerPart, i32FractionPart);

	post_len = strlen(post_str);
	post_str[post_len] = '&';
	post_len++;

    //
    // Print temperature with three digits of decimal precision.
    //
    //UARTprintf(" Temperature %3d.%03d\n", i32IntegerPart,
    //           i32FractionPart);

    //
    // Convert the pressure to an integer part and fraction part for
    // easy print.
    //
    i32IntegerPart = (int32_t) fPressure;
    i32FractionPart =(int32_t) (fPressure * 1000.0f);
    i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
    if(i32FractionPart < 0)
    {
        i32FractionPart *= -1;
    }

	memcpy(&post_str[post_len], BMPP_ALIAS, BMP_ALIAS_LENGTH);
	post_len += strlen(BMPP_ALIAS);
	post_str[post_len] = '=';
	post_len++;
	post_len += snprintf(&post_str[post_len], 10, "%2d.%02d", i32IntegerPart, i32FractionPart);

	post_len = strlen(post_str);
	post_str[post_len] = '&';
	post_len++;

    //
    // Print Pressure with three digits of decimal precision.
    //
    //UARTprintf(" Pressure %3d.%03d\n", i32IntegerPart, i32FractionPart);

    //
    // Calculate the altitude.
    //
    //fAltitude = 44330.0f * (1.0f - powf(fPressure / 101325.0f,
    //                                    1.0f / 5.255f));

    //
    // Convert the altitude to an integer part and fraction part for easy
    // print.
    //
    //i32IntegerPart = (int32_t) fAltitude;
    //i32FractionPart =(int32_t) (fAltitude * 1000.0f);
    //i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
    //if(i32FractionPart < 0)
    //{
    //    i32FractionPart *= -1;
    //}

    //
    // Print altitude with three digits of decimal precision.
    //
    //UARTprintf(" Altitude %3d.%03d", i32IntegerPart, i32FractionPart);

    //
    // Print new line.
    //
    //UARTprintf("\n");
}

void readSht21Data(void)
{
    float fTemperature, fHumidity;
    int32_t i32IntegerPart;
    int32_t i32FractionPart;

    //
    // Write the command to start a humidity measurement.
    //
    SHT21Write(&g_sSHT21Inst, SHT21_CMD_MEAS_RH, g_sSHT21Inst.pui8Data, 0,
            SHT21AppCallback, &g_sSHT21Inst);

    //
    // Wait for the I2C transactions to complete before moving forward.
    //
    SHT21AppI2CWait(__FILE__, __LINE__);

    //
    // Wait 33 milliseconds before attempting to get the result. Datasheet
    // claims this can take as long as 29 milliseconds.
    //
    ROM_SysCtlDelay(ui32SysClock / (30 * 3));

    //
    // Get the raw data from the sensor over the I2C bus.
    //
    SHT21DataRead(&g_sSHT21Inst, SHT21AppCallback, &g_sSHT21Inst);

    //
    // Wait for the I2C transactions to complete before moving forward.
    //
    SHT21AppI2CWait(__FILE__, __LINE__);

    //
    // Get a copy of the most recent raw data in floating point format.
    //
    SHT21DataHumidityGetFloat(&g_sSHT21Inst, &fHumidity);

    //
    // Write the command to start a temperature measurement.
    //
    SHT21Write(&g_sSHT21Inst, SHT21_CMD_MEAS_T, g_sSHT21Inst.pui8Data, 0,
            SHT21AppCallback, &g_sSHT21Inst);

    //
    // Wait for the I2C transactions to complete before moving forward.
    //
    SHT21AppI2CWait(__FILE__, __LINE__);

    //
    // Wait 100 milliseconds before attempting to get the result. Datasheet
    // claims this can take as long as 85 milliseconds.
    //
    ROM_SysCtlDelay(ui32SysClock / (10 * 3));

    //
    // Read the conversion data from the sensor over I2C.
    //
    SHT21DataRead(&g_sSHT21Inst, SHT21AppCallback, &g_sSHT21Inst);

    //
    // Wait for the I2C transactions to complete before moving forward.
    //
    SHT21AppI2CWait(__FILE__, __LINE__);

    //
    // Get the most recent temperature result as a float in celcius.
    //
    SHT21DataTemperatureGetFloat(&g_sSHT21Inst, &fTemperature);

    //
    // Convert the floats to an integer part and fraction part for easy
    // print. Humidity is returned as 0.0 to 1.0 so multiply by 100 to get
    // percent humidity.
    //
    fHumidity *= 100.0f;
    i32IntegerPart = (int32_t) fHumidity;
    i32FractionPart = (int32_t) (fHumidity * 1000.0f);
    i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
    if(i32FractionPart < 0)
    {
        i32FractionPart *= -1;
    }

	memcpy(&post_str[post_len], SHTH_ALIAS, SHT_ALIAS_LENGTH);
	post_len += strlen(SHTH_ALIAS);
	post_str[post_len] = '=';
	post_len++;
	post_len += snprintf(&post_str[post_len], 10, "%2d.%02d", i32IntegerPart, i32FractionPart);

	post_len = strlen(post_str);
	post_str[post_len] = '&';
	post_len++;
    //
    // Print the humidity value using the integers we just created.
    //
    //UARTprintf(" Humidity %3d.%03d\n", i32IntegerPart, i32FractionPart);

    //
    // Perform the conversion from float to a printable set of integers.
    //
    i32IntegerPart = (int32_t) fTemperature;
    i32FractionPart = (int32_t) (fTemperature * 1000.0f);
    i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
    if(i32FractionPart < 0)
    {
        i32FractionPart *= -1;
    }

	memcpy(&post_str[post_len], SHTT_ALIAS, SHT_ALIAS_LENGTH);
	post_len += strlen(SHTT_ALIAS);
	post_str[post_len] = '=';
	post_len++;
	post_len += snprintf(&post_str[post_len], 10, "%2d.%02d", i32IntegerPart, i32FractionPart);

	post_len = strlen(post_str);
	post_str[post_len] = '&';
	post_len++;

    //
    // Print the temperature as integer and fraction parts.
    //
    //UARTprintf(" Temperature %3d.%03d\n", i32IntegerPart, i32FractionPart);

    //
    // Delay for one second. This is to keep sensor duty cycle
    // to about 10% as suggested in the datasheet, section 2.4.
    // This minimizes self heating effects and keeps reading more accurate.
    //
    ROM_SysCtlDelay(ui32SysClock / 3);
}

void readIsl29023Data(void)
{
    float fAmbient;
    int32_t i32IntegerPart, i32FractionPart;

    //
    // Wait for the DataFlag which is set when a DataRead is complete.
    // DataRead is started in the SysTick Interrupt Handler.
    if(g_vui8DataFlag)
    {
        g_vui8DataFlag = 0;

        //
        // Get a local floating point copy of the latest light data
        //
        ISL29023DataLightVisibleGetFloat(&g_sISL29023Inst, &fAmbient);

        //
        // Perform the conversion from float to a printable set of integers
        //
        i32IntegerPart = (int32_t)fAmbient;
        i32FractionPart = (int32_t)(fAmbient * 1000.0f);
        i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
        if(i32FractionPart < 0)
        {
            i32FractionPart *= -1;
        }

    	memcpy(&post_str[post_len], ISL_ALIAS, ISL_ALIAS_LENGTH);
    	post_len += strlen(ISL_ALIAS);
    	post_str[post_len] = '=';
    	post_len++;
    	post_len += snprintf(&post_str[post_len], 10, "%2d.%02d", i32IntegerPart, i32FractionPart);


        //
        // Print the temperature as integer and fraction parts.
        //
        //UARTprintf("Visible Lux: %3d.%03d\n", i32IntegerPart,
        //           i32FractionPart);

        //
        // Check if the intensity of light has crossed a threshold. If so
        // then adjust range of sensor readings to track intensity.
        //
        if(g_vui8IntensityFlag)
        {
            //
            // Disable the low priority interrupts leaving only the I2C
            // interrupt enabled.
            //
            ROM_IntPriorityMaskSet(0x40);

            //
            // Reset the intensity trigger flag.
            //
            g_vui8IntensityFlag = 0;

            //
            // Adjust the lux range.
            //
            ISL29023AppAdjustRange(&g_sISL29023Inst);

            //
            // Now we must manually clear the flag in the ISL29023
            // register.
            //
            ISL29023Read(&g_sISL29023Inst, ISL29023_O_CMD_I,
                         g_sISL29023Inst.pui8Data, 1, ISL29023AppCallback,
                         &g_sISL29023Inst);

            //
            // Wait for transaction to complete
            //
            ISL29023AppI2CWait(__FILE__, __LINE__);

            //
            // Disable priority masking so all interrupts are enabled.
            //
            ROM_IntPriorityMaskSet(0);
        }
    }
}

//*****************************************************************************
//
// Intensity and Range Tracking Function.  This adjusts the range and interrupt
// thresholds as needed.  Uses an 80/20 rule. If light is greather then 80% of
// maximum value in this range then go to next range up. If less than 20% of
// potential value in this range go the next range down.
//
//*****************************************************************************
void
ISL29023AppAdjustRange(tISL29023 *pInst)
{
    float fAmbient;
    uint8_t ui8NewRange;

    ui8NewRange = g_sISL29023Inst.ui8Range;

    //
    // Get a local floating point copy of the latest light data
    //
    ISL29023DataLightVisibleGetFloat(&g_sISL29023Inst, &fAmbient);

    //
    // Check if we crossed the upper threshold.
    //
    if(fAmbient > g_fThresholdHigh[g_sISL29023Inst.ui8Range])
    {
        //
        // The current intensity is over our threshold so adjsut the range
        // accordingly
        //
        if(g_sISL29023Inst.ui8Range < ISL29023_CMD_II_RANGE_64K)
        {
            ui8NewRange = g_sISL29023Inst.ui8Range + 1;
        }
    }

    //
    // Check if we crossed the lower threshold
    //
    if(fAmbient < g_fThresholdLow[g_sISL29023Inst.ui8Range])
    {
        //
        // If possible go to the next lower range setting and reconfig the
        // thresholds.
        //
        if(g_sISL29023Inst.ui8Range > ISL29023_CMD_II_RANGE_1K)
        {
            ui8NewRange = g_sISL29023Inst.ui8Range - 1;
        }
    }

    //
    // If the desired range value changed then send the new range to the sensor
    //
    if(ui8NewRange != g_sISL29023Inst.ui8Range)
    {
        ISL29023ReadModifyWrite(&g_sISL29023Inst, ISL29023_O_CMD_II,
                                ~ISL29023_CMD_II_RANGE_M, ui8NewRange,
                                ISL29023AppCallback, &g_sISL29023Inst);

        //
        // Wait for transaction to complete
        //
        ISL29023AppI2CWait(__FILE__, __LINE__);
    }
}
