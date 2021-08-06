/*
 * See app note:
 * https://www.melexis.com/en/documents/documentation/application-notes/application-note-mlx90614-changing-emissivity-setting
 *
 * 1. Write 0x0000 to address 0x04 (erase the EEPROM cell)
 * 2. Write the new value to address 0x04
 * 3. Read the value in address 0x04 in order to check that the correct value is stored
 * 4. Restart the module
 *
 */

#include <Adafruit_MLX90614.h>

//== CHANGE THIS ============
double new_emissivity = 0.95;
//===========================

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Adafruit MLX90614 Emissivity Setter.\n");

  // init sensor
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };

  // read current emissivity
  Serial.print("Current emissivity = "); Serial.println(mlx.readEmissivity());

  // set new emissivity
  Serial.print("Setting emissivity = "); Serial.println(new_emissivity);
  mlx.writeEmissivity(new_emissivity); // this does the 0x0000 erase write

  // read back
  Serial.print("New emissivity = "); Serial.println(mlx.readEmissivity());

  // done
  Serial.print("DONE. Restart the module.");
}

void loop() {
}