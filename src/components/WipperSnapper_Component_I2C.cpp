/*!
 * @file Wippersnapper_Component_I2C.cpp
 *
 * This component initiates I2C operations
 * using the Arduino generic TwoWire driver.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Copyright (c) Brent Rubell 2021 for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "WipperSnapper_Component_I2C.h"

/***************************************************************************************************************/
/*!
    @brief    Creates a new WipperSnapper I2C component.
    @param    msgInitRequest
              The I2C initialization request message.
*/
/***************************************************************************************************************/
WipperSnapper_Component_I2C::WipperSnapper_Component_I2C(
    wippersnapper_i2c_v1_I2CInitRequest *msgInitRequest) {
  WS_DEBUG_PRINTLN("NEW WipperSnapper_Component_I2C");
  // initialize using desired portNum
  _i2c = new TwoWire(msgInitRequest->i2c_port_number);
  // validate if SDA & SCL has pullup
  if (digitalRead(msgInitRequest->i2c_pin_sda) == LOW) {
    pinMode(msgInitRequest->i2c_pin_sda, INPUT_PULLUP);
  }
  if (digitalRead(msgInitRequest->i2c_pin_scl) == LOW) {
    pinMode(msgInitRequest->i2c_pin_scl, INPUT_PULLUP);
  }
  // set up i2c port
  _i2c->begin(msgInitRequest->i2c_pin_sda, msgInitRequest->i2c_pin_scl);
  _i2c->setClock(msgInitRequest->i2c_frequency);
  WS_DEBUG_PRINTLN("Init. new I2C Port: ");
  WS_DEBUG_PRINT("Port#: ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_port_number);
  WS_DEBUG_PRINT("SDA: ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_pin_sda);
  WS_DEBUG_PRINT("SCL: ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_pin_scl);
  WS_DEBUG_PRINT("Frequency (Hz): ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_frequency);
  // set i2c obj. properties
  _portNum = msgInitRequest->i2c_port_number;
  _isInit = true;
  yield();
}

/*************************************************************/
/*!
    @brief    Destructor for a WipperSnapper I2C component.
*/
/*************************************************************/
WipperSnapper_Component_I2C::~WipperSnapper_Component_I2C() {
  _portNum = 100; // Invalid = 100
  _isInit = false;
}

/*****************************************************************************************************/
/*!
    @brief    Destructor for a WipperSnapper I2C component.
    @param    msgScanReq
              A decoded I2C scan request message.
    @returns  The address which an I2C device is located, -1 otherwise.
*/
/*****************************************************************************************************/
uint16_t WipperSnapper_Component_I2C::scanAddresses(
    wippersnapper_i2c_v1_I2CScanRequest msgScanReq) {
  // decode stream into i2c request
  uint16_t addrFound = -1;
  uint16_t scanAddr;
  WS_DEBUG_PRINT("EXEC: I2C Scan, Port (");
  WS_DEBUG_PRINT(_portNum);
  WS_DEBUG_PRINTLN(")");
  for (int i = 0; i < msgScanReq.address_count; i++) {
    scanAddr = msgScanReq.address[i];
    WS_DEBUG_PRINT("Scanning address ");
    WS_DEBUG_PRINTLN(scanAddr);
    _i2c->beginTransmission(scanAddr);
    if (_i2c->endTransmission() == 0) {
      // found it!
      WS_DEBUG_PRINTLN("I2C device detected!");
      addrFound = scanAddr;
      break;
    } else {
      WS_DEBUG_PRINTLN("I2C device not detected!");
    }
  }
  return addrFound;
}


/*******************************************************************************/
/*!
    @brief    Initializes I2C device driver and attaches its object to the "bus"
    @param    msgDeviceInitReq
              A decoded I2CDevice initialization request message.
    @returns True if I2C device is initialized and attached, False otherwise.
*/
/*******************************************************************************/
bool WipperSnapper_Component_I2C::attachI2CDevice(wippersnapper_i2c_v1_I2CDeviceInitRequest *msgDeviceInitReq) {
  bool attachSuccess = false;
  // Determine which sensor-specific callback to utilize

  // AHTX0 Sensor
  if (msgDeviceInitReq->has_aht_init) {
      uint16_t addr = (uint16_t) msgDeviceInitReq->aht_init.address;
      WS_DEBUG_PRINTLN("Requesting to initialize AHTx sensor");
      WS_DEBUG_PRINT("\tSensor Addr: ");WS_DEBUG_PRINTLN(addr);
      WS_DEBUG_PRINT("\tTemperature sensor enabled? ");WS_DEBUG_PRINTLN(msgDeviceInitReq->aht_init.enable_temperature);
      WS_DEBUG_PRINT("\tHumidity sensor enabled? ");WS_DEBUG_PRINTLN(msgDeviceInitReq->aht_init.enable_humidity);

      // TODO: Create I2C Driver using the an AHT driver sub-class!
       I2C_Driver * aht = new I2C_Driver(addr, this->_i2c);
      // Attempt to initialize the sensor driver
      if (aht->initAHTX0()) {
          attachSuccess = false;
          return attachSuccess;
      }
      // Initialize device-specific sensors
      if (msgDeviceInitReq->aht_init.enable_temperature == true) {
          aht->enableAHTX0Temperature();
      }
      if (msgDeviceInitReq->aht_init.enable_humidity == true) {
          aht->enableAHTX0Humidity();
      }
      // Push to vector for sensor drivers
      activeDrivers.push_back(aht);
      attachSuccess = true; 
  } else {
    WS_DEBUG_PRINTLN("ERROR: Sensor not found")
  }
  WS_DEBUG_PRINTLN("Successfully initialized AHTX0 sensor!");
  return attachSuccess;
}