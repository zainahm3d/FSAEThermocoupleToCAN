/**
 *  @brief 5ch MAX31855K thermocouple amp to CAN firmware
 *
 *  CAN data frame Layout:
 *  Bytes 0 - 1 uint16 encoded an0 Data
 *  Bytes 2 - 3 uint16 encoded an1 Data
 *  CAN Packet size: 4 bytes
*/

#include "main.h"
#include "max31855.h"
#include <mbed.h>

int main() {

  setupGPIOs();
  setupCAN();
  blinkLEDs();

  maxSPI.frequency(SPI_FREQ);
  // initialize all MAX31855K interfaces
  for (int i = 0; i < NUM_CHANNELS; i++) {
    maxArray[i].initialise();
  }

  pwrLED.write(1);

  timer.reset();
  timer.start();

  while (1) {
    can.read(inMsg); // read and throw away input messages to clear buffer

    if (timer.read_ms() >= CAN_MSG_PERIOD_MS) {
      readThermocouples();
      sendCANmessages();

      timer.reset();
      timer.start();
    }
  }
}

/// @brief Ensure all CS pins are deasserted
void setupGPIOs() {
  for (int i = 0; i < NUM_CHANNELS; i++) {
    CSArray[i].write(1); // deassert chip select
  }
}

/// @brief Set CAN bitrate and setup messages
void setupCAN() {
  can.frequency(CAN_BAUD);

  outMsgA.id = outMsgA_ID;
  outMsgB.id = outMsgB_ID;

  outMsgA.format = CANStandard;
  outMsgB.format = CANStandard;

  outMsgA.len = 8;
  outMsgB.len = 2;
}

/// @brief Reads each thermocouple channel and populates output array
void readThermocouples() {

  for (int i = 0; i < NUM_CHANNELS; i++) { // loop through all MAX31855K thermocouple amps

    // get value

    if (maxArray[i].ready()) {
      uint16_t tempBits = maxArray[i].read_temp();

      if (tempBits < 2000 && tempBits > 0) {
        // value is good
        tempArray[i] = ((tempBits * (9.0 / 5.0)) + 32.0); // convert value to F
        ledArray[i].write(1);

      } else {
        // value is bad
        // set breakpoints here to find issues
        switch (tempBits) {
          case MAX31855_ERR_NO_TC:
            ledArray[i].write(0); // turn off LED corresponding to thermocouple
            break;

          case MAX31855_ERR_SHORT_GND:
            ledArray[i].write(0);
            break;

          case MAX31855_ERR_SHORT_VCC:
            ledArray[i].write(0);
            break;

          default:
            ledArray[i].write(0);
            break;
        }

        // indicate error by setting value to 0
        tempArray[i] = 0;
      }
    }
  }
}

/// @brief Fill and send CAN messages, MSByte first
void sendCANmessages() {
  outMsgA.data[0] = (tempArray[0] >> 8) & 0xFF;
  outMsgA.data[1] = tempArray[0] & 0xFF;

  outMsgA.data[2] = (tempArray[1] >> 8) & 0xFF;
  outMsgA.data[3] = tempArray[1] & 0xFF;

  outMsgA.data[4] = (tempArray[2] >> 8) & 0xFF;
  outMsgA.data[5] = tempArray[2] & 0xFF;

  outMsgA.data[6] = (tempArray[3] >> 8) & 0xFF;
  outMsgA.data[7] = tempArray[3] & 0xFF;

  outMsgB.data[0] = (tempArray[4] >> 8) & 0xFF;
  outMsgB.data[1] = tempArray[4] & 0xFF;

  // If message is successfully pushed to CAN FIFO, toggle LED.
  if (can.write(outMsgA) && can.write(outMsgB)) {
    txLED = !txLED;
  } else {
    txLED = 0;
  }
}

/// @brief Check if all LEDs are alive. Also be obnoxious.
void blinkLEDs() {
  pwrLED.write(1);
  wait_ms(250);
  txLED.write(1);
  wait_ms(250);

  for (int i = 0; i < NUM_CHANNELS; i++) {
    ledArray[i].write(1);
    wait_ms(250);
  }

  wait_ms(2000);
}