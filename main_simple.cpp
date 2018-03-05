/*******************************************************************************
* Copyright (C) 2018 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/

#include "mbed.h"
#include "OneWire.h"
#include "max32630fthr.h"
#include "USBSerial.h"

//LSM9DS0 accelerometer definitions
#define MAGX_WADDR          0x3A  // I2C Write Address for accelerometer
#define CTRL_REG1_XM        0x20  // Sample rate and axis enable register
#define OUT_X_L_A           0xA8  // Accelerometer x-axis output LSB


using namespace OneWire;
using namespace RomCommands;

//Set IO logic level to 3.3V
MAX32630FTHR pegasus(MAX32630FTHR::VIO_3V3);

//Virtual serial port over USB
USBSerial pc; 

//Indicator LED
DigitalOut gLED(LED2, LED_ON);

//------------------------------------------------------------------------------
int main()
{
    //Set up OneWire master
    MCU_OWM owm(false, false);
    owm.OWInitMaster();
    
    //Set up DS28E17 slave device
    SearchState search_state;            //State used for 1-Wire searches
    search_state.findFamily(0x19);       //Set family code for the DS28E17
    OWNext(owm, search_state);           //Find the 1-Wire slave romId
    
    MultidropRomIterator selector(owm);  //Object for a general 1-Wire slave
    DS28E17 bridge(selector);            //DS28E17 constructor
    bridge.setRomId(search_state.romId); //Assign the romId to DS28E17 object
    
    //LSM9DS0 setup
    uint8_t status, wr_status;
    uint8_t writeData[2] = {CTRL_REG1_XM, 0x67}; //100sps, all axes enabled
    bridge.writeDataWithStop(MAGX_WADDR, 2, writeData, status, wr_status);
    
    uint8_t readData[6];
    int16_t rawX;
    int16_t rawY;
    int16_t rawZ;
    
    //Loop to read accelerometer and send serial data
    //--------------------------------------------------------------------------
    while(1)
    {
        //Read 6 bytes from LSM6DS0 accelerometer, starting at OUT_X_L_A
        writeData[0] = OUT_X_L_A;
        bridge.writeReadDataWithStop(MAGX_WADDR, 1, writeData, 6, status, wr_status, readData);
        
        //Parse read data into x-y-z values
        rawX = readData[1] << 8 | readData[0];
        rawY = readData[3] << 8 | readData[2];
        rawZ = readData[5] << 8 | readData[4];
        
        //Print using Arduino Serial Plotter format
        pc.printf("%d %d %d\r\n", rawX, rawY, rawZ);

        //Indicate activity
        gLED = !gLED; 
    }
}