/***************************************************
  This is a library for the MLX90614 Temp Sensor

  Designed specifically to work with the MLX90614 sensors in the
  adafruit shop
  ----> https://www.adafruit.com/products/1747 (3V)
  ----> https://www.adafruit.com/products/1748 (5V)

  These sensors use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_MLX90614.h"

/**
 * @brief Begin the I2C connection
 * @param addr I2C address for the device.
 * @param wire Pointer to Wire instance
 * @return True if the device was successfully initialized, otherwise false.
 */
bool Adafruit_MLX90614::begin(uint8_t addr, TwoWire *wire) {
  _addr = addr; // needed for CRC
  i2c_dev = new Adafruit_I2CDevice(addr, wire);
  return i2c_dev->begin();
}

/**
 * @brief Read the raw value from the emissivity register
 *
 * @return uint16_t The unscaled emissivity value or '0' if reading failed
 */
uint16_t Adafruit_MLX90614::readEmissivityReg(void) {
  return read16(MLX90614_EMISS);
}
/**
 * @brief Write the raw unscaled emissivity value to the emissivity register
 *
 * @param ereg The unscaled emissivity value
 */
void Adafruit_MLX90614::writeEmissivityReg(uint16_t ereg) {
  write16(MLX90614_EMISS, 0); // erase
  delay(10);
  write16(MLX90614_EMISS, ereg);
  delay(10);
}
/**
 * @brief Read the emissivity value from the sensor's register and scale
 *
 * @return double The emissivity value, ranging from 0.1 - 1.0 or NAN if reading
 * failed
 */
double Adafruit_MLX90614::readEmissivity(void) {
  uint16_t ereg = read16(MLX90614_EMISS);
  if (ereg == 0)
    return NAN;
  return ((double)ereg) / 65535.0;
}
/**
 * @brief Set the emissivity value
 *
 * @param emissivity The emissivity value to use, between 0.1 and 1.0
 */
void Adafruit_MLX90614::writeEmissivity(double emissivity) {
  uint16_t ereg = (uint16_t)(0xffff * emissivity);

  writeEmissivityReg(ereg);
}

/**
 * @brief Read the raw value from the Config Register1
 *
 * @return uint16_t The Config Register1 value or '0' if reading failed
 */
uint16_t Adafruit_MLX90614::readConfigReg(void) {
  return read16(MLX90614_CONFIG);
}
/**
 * @brief Write the raw value to the Config Register1
 *
 * @param creg The Config Register1 value
 */
void Adafruit_MLX90614::writeConfigReg(uint16_t creg) {
  write16(MLX90614_CONFIG, creg);
  delay(10);
}

/**
 * @brief Read the FIR filter value from the Config Register1
 *
 * @return uint16_t The FIR filter value or NOISE_FILTER_ERROR on error occur
 */
Adafruit_MLX90614::noise_filter_mode Adafruit_MLX90614::readNoiseFilterMode(void) {
  uint16_t reg = readConfigReg();
  if (reg == 0) {
    return NOISE_FILTER_ERROR;
  }
  return noise_filter_mode(reg & 0x0700);
}
/**
 * @brief Set the FIR filter value to the Config Register1
 *
 * @param mode The FIR filter value to use
 */
void Adafruit_MLX90614::writeNoiseFilterMode(noise_filter_mode mode) {
  if (mode < NOISE_LIMIT_128 || mode > NOISE_LIMIT_1024) {
    return;
  }

  uint16_t reg = readConfigReg();
  if (reg == 0 || (reg & 0x0700) == mode /* save EEPROM erase-write cycles */) {
    return;
  }

  writeConfigReg(0); // erase
  writeConfigReg((reg & ~0x0700) | mode);
}

/**
 * @brief Read the IIR filter value from the Config Register1
 *
 * @return uint16_t The IIR filter value or SPIKE_FILTER_ERROR on error occur
 */
Adafruit_MLX90614::spike_filter_mode Adafruit_MLX90614::readSpikeFilterMode(void) {
  uint16_t reg = readConfigReg();
  if (reg == 0) {
    return SPIKE_FILTER_ERROR;
  }
  return spike_filter_mode(reg & 0x0007);
}
/**
 * @brief Set the IIR filter value
 *
 * @param mode The IIR filter value to use
 */
void Adafruit_MLX90614::writeSpikeFilterMode(spike_filter_mode mode) {
  if (mode > 0x0007) {
    return;
  }

  uint16_t reg = readConfigReg();
  if (reg == 0 || (reg & 0x0007) == mode /* save EEPROM erase-write cycles */) {
    return;
  }

  writeConfigReg(0); // erase
  writeConfigReg((reg & ~0x0007) | mode);
}

/**
 * @brief Set FIR/IIR filter values
 *
 * @param noise The FIR filter value to use
 * @param spike The IIR filter value to use
 */
void Adafruit_MLX90614::writeFilterMode(noise_filter_mode noise, spike_filter_mode spike) {
  if (noise < NOISE_LIMIT_128 || noise > NOISE_LIMIT_1024 || spike > 0x0007) {
    return;
  }

  uint16_t reg = readConfigReg();
  if (reg == 0 || ((reg & 0x0700) == noise && (reg & 0x0007) == spike) /* save EEPROM erase-write cycles */) {
    return;
  }

  writeConfigReg(0); // erase
  writeConfigReg(((reg & ~0x0007) & ~0x0700) | noise | spike);
}

/**
 * @brief Return settling time in ms based on FIR/IIR filter values
 * See Application Note Understanding MLX90614 on-chip digital signal filters
 *
 * @return uint16_t The settling time in ms or 0 on error occur
 */
unsigned long Adafruit_MLX90614::settlingTime(void) {
  uint16_t reg = readConfigReg();
  if (reg == 0) {
    return 0;
  }

  double fir;
  switch (reg & 0x0700) {
    case NOISE_LIMIT_128:  fir = 5.184;  break;
    case NOISE_LIMIT_256:  fir = 9.280;  break;
    case NOISE_LIMIT_512:  fir = 17.472; break;
    case NOISE_LIMIT_1024: fir = 33.856; break;
    default:
      return 0;
  }

  double iir;
  switch (reg & 0x0007) {
    case SPIKE_LIMIT_13:
    case SPIKE_LIMIT_17:
    case SPIKE_LIMIT_25:
    case SPIKE_LIMIT_50:
      iir = 10;
      break;
    case SPIKE_LIMIT_57:  iir = 9; break;
    case SPIKE_LIMIT_67:  iir = 8; break;
    case SPIKE_LIMIT_80:  iir = 4; break;
    case SPIKE_LIMIT_100: iir = 1; break;
    default:
      return 0;
  }

  uint16_t dual = (reg & 0x0040) >> 6;

  return 9.719 + (iir * (fir + 5.26)) + (iir * (fir + 12.542)) + (iir * dual * (fir + 12.542));
}


/**
 * @brief Get the current temperature of an object in degrees Farenheit
 *
 * @return double The temperature in degrees Farenheit or NAN if reading failed
 */
double Adafruit_MLX90614::readObjectTempF(void) {
  return (readTemp(MLX90614_TOBJ1) * 9 / 5) + 32;
}
/**
 * @brief Get the current ambient temperature in degrees Farenheit
 *
 * @return double The temperature in degrees Farenheit or NAN if reading failed
 */
double Adafruit_MLX90614::readAmbientTempF(void) {
  return (readTemp(MLX90614_TA) * 9 / 5) + 32;
}

/**
 * @brief Get the current temperature of an object in degrees Celcius
 *
 * @return double The temperature in degrees Celcius or NAN if reading failed
 */
double Adafruit_MLX90614::readObjectTempC(void) {
  return readTemp(MLX90614_TOBJ1);
}

/**
 * @brief Get the current ambient temperature in degrees Celcius
 *
 * @return double The temperature in degrees Celcius or NAN if reading failed
 */
double Adafruit_MLX90614::readAmbientTempC(void) {
  return readTemp(MLX90614_TA);
}

float Adafruit_MLX90614::readTemp(uint8_t reg) {
  float temp;

  temp = read16(reg);
  if (temp == 0)
    return NAN;
  temp *= .02;
  temp -= 273.15;
  return temp;
}

/*********************************************************************/

uint16_t Adafruit_MLX90614::read16(uint8_t a) {
  uint8_t buffer[3];
  buffer[0] = a;
  // read two bytes of data + pec
  bool status = i2c_dev->write_then_read(buffer, 1, buffer, 3);
  if (!status)
    return 0;
  // return data, ignore pec
  return uint16_t(buffer[0]) | (uint16_t(buffer[1]) << 8);
}

byte Adafruit_MLX90614::crc8(byte *addr, byte len)
// The PEC calculation includes all bits except the START, REPEATED START, STOP,
// ACK, and NACK bits. The PEC is a CRC-8 with polynomial X8+X2+X1+1.
{
  byte crc = 0;
  while (len--) {
    byte inbyte = *addr++;
    for (byte i = 8; i; i--) {
      byte carry = (crc ^ inbyte) & 0x80;
      crc <<= 1;
      if (carry)
        crc ^= 0x7;
      inbyte <<= 1;
    }
  }
  return crc;
}

void Adafruit_MLX90614::write16(uint8_t a, uint16_t v) {
  uint8_t buffer[4];

  buffer[0] = _addr << 1;
  buffer[1] = a;
  buffer[2] = v & 0xff;
  buffer[3] = v >> 8;

  uint8_t pec = crc8(buffer, 4);

  buffer[0] = buffer[1];
  buffer[1] = buffer[2];
  buffer[2] = buffer[3];
  buffer[3] = pec;

  i2c_dev->write(buffer, 4);
}
