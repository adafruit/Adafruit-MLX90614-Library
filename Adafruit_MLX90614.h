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

  Written by Limor Fried/Ladyada for Adafruit in any redistribution
 ****************************************************/

#include <Adafruit_I2CDevice.h>
#include <Arduino.h>

#define MLX90614_I2CADDR 0x5A

// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x2E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F

/**
 * @brief Class to read from and control a MLX90614 Temp Sensor
 *
 */
class Adafruit_MLX90614 {
public:
  /**
   * @brief FIR filter mode (limit the noise)
   */
  enum noise_filter_mode {
    NOISE_LIMIT_128    = 0b10000000000, ///< Low noise filter, FIR = 128
    NOISE_LIMIT_256    = 0b10100000000, ///< FIR = 256
    NOISE_LIMIT_512    = 0b11000000000, ///< FIR = 512
    NOISE_LIMIT_1024   = 0b11100000000, ///< Max noise filter, FIR = 1024, default
    NOISE_FILTER_ERROR = 0              ///< Error value
  };

  /**
   * @brief IIR filter mode (limit the sharpness / spikes)
   */
  enum spike_filter_mode {
    SPIKE_LIMIT_100    = 0b100, ///< No spike reduction, a1 = 1, b1 = 0
    SPIKE_LIMIT_80     = 0b101, ///< 80% default spike filter, a1 = 0.8, b1 = 0.2
    SPIKE_LIMIT_67     = 0b110, ///< 67% spike filter, a1 = 0.666, b1 = 0.333
    SPIKE_LIMIT_57     = 0b111, ///< 57% spike filter, a1 = 0.571, b1 = 0.428
    SPIKE_LIMIT_50     = 0b000, ///< 50% spike filter, a1 = 0.5, b1 = 0.5
    SPIKE_LIMIT_25     = 0b001, ///< 25% spike filter, a1 = 0.25, b1 = 0.75
    SPIKE_LIMIT_17     = 0b010, ///< 17% spike filter, a1 = 0.1666(6), b1 = 0.83(3)
    SPIKE_LIMIT_13     = 0b011, ///< 13% maximum spike reduction, a1 = 0.125, b1 = 0.875
    SPIKE_FILTER_ERROR = 0b1000 ///< Error value
  };

  bool begin(uint8_t addr = MLX90614_I2CADDR, TwoWire *wire = &Wire);

  double readObjectTempC(void);
  double readAmbientTempC(void);
  double readObjectTempF(void);
  double readAmbientTempF(void);
  uint16_t readEmissivityReg(void);
  void writeEmissivityReg(uint16_t ereg);
  double readEmissivity(void);
  void writeEmissivity(double emissivity);

  uint16_t readConfigReg(void);
  void writeConfigReg(uint16_t creg);

  noise_filter_mode readNoiseFilterMode(void);
  void writeNoiseFilterMode(noise_filter_mode mode);

  spike_filter_mode readSpikeFilterMode(void);
  void writeSpikeFilterMode(spike_filter_mode mode);

  void writeFilterMode(noise_filter_mode noise, spike_filter_mode spike);

  unsigned long settlingTime(void);

private:
  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
  float readTemp(uint8_t reg);

  uint16_t read16(uint8_t addr);
  void write16(uint8_t addr, uint16_t data);
  byte crc8(byte *addr, byte len);
  uint8_t _addr;
};
