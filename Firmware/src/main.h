#ifndef MAIN_H
#define MAIN_H

#include "max31855.h"
#include <mbed.h>

#define CAN_MSG_PERIOD_MS 1000
#define CAN_BAUD 250000

// Since we can only fit 4 16 bit numbers per frame and keep formatting clean, we need 2 frames for 5 thermocouples
#define outMsgA_ID 0x1E
#define outMsgB_ID 0x1F

#define SPI_FREQ 1000000

#define NUM_CHANNELS 5

// MAX31855 error status codes
#define MAX31855_ERR_NO_TC 2001
#define MAX31855_ERR_SHORT_GND 2002
#define MAX31855_ERR_SHORT_VCC 2004

// Set up IOs
CAN can(PA_11, PA_12);

// Board status LEDs
DigitalOut pwrLED(PA_1);
DigitalOut txLED(PA_4);

// Thermocouple status LEDs
DigitalOut ch1LED(PB_1);
DigitalOut ch2LED(PB_0);
DigitalOut ch3LED(PA_7);
DigitalOut ch4LED(PA_6);
DigitalOut ch5LED(PA_5);

// CS Pins
DigitalOut cs1(PA_10);
DigitalOut cs2(PA_9);
DigitalOut cs3(PB_11);
DigitalOut cs4(PB_10);
DigitalOut cs5(PB_2);

// MAX31855K SPI interfaces
// Bug in current version of MBED requires MOSI be set (would use NC)
SPI maxSPI(PB_15, PB_14, PB_13);

// MAX31855K library initialization
max31855 max1(maxSPI, PA_10);
max31855 max2(maxSPI, PA_9);
max31855 max3(maxSPI, PB_11);
max31855 max4(maxSPI, PB_10);
max31855 max5(maxSPI, PB_2);

// Software timer
Timer timer;

// CAN messages
CANMessage outMsgA; // channels 1 - 4
CANMessage outMsgB; // channel 5
CANMessage inMsg;   // used to dump incoming frames

// Allows for looping through CS pins
DigitalOut CSArray[NUM_CHANNELS] = {cs1, cs2, cs3, cs4, cs5};

// Allows for looping through status LEDs
DigitalOut ledArray[NUM_CHANNELS] = {ch1LED, ch2LED, ch3LED, ch4LED, ch5LED};

// Allows for looping through MAX31855K modules
max31855 maxArray[NUM_CHANNELS] = {max1, max2, max3, max4, max5};

// Holds temperature or status values
uint16_t tempArray[NUM_CHANNELS] = {0};

// Function Prototypes
void setupGPIOs();
void setupCAN();
void readThermocouples();
void sendCANmessages();
void blinkLEDs();

#endif // MAIN_H