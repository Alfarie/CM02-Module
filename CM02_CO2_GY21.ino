#include <Wire.h>
#include <LibHumidity.h>
#include <SoftwareSerial.h>

int co2Addr = 0x68;

SoftwareSerial mySerial(8, 9);
LibHumidity humidity = LibHumidity(0);

float temp_sensor,humi_sensor;
int co2_temp,co2_sensor;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin ();
  mySerial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  getGy21();
  getK30();

  String sensorStr = "{cm02" + String(temp_sensor) + "," + String(humi_sensor)+","+String(co2_sensor) + "}";
  mySerial.println(sensorStr);
  
//  Serial.println(sensorStr);
  
  delay(1000);
}

void getGy21(){
  temp_sensor = humidity.GetTemperatureC();
  humi_sensor = humidity.GetHumidity();
}

void getK30(){
  co2_temp = co2_sensor;
  int r = readCO2();
  co2_sensor = ( r > 0)?r:co2_temp;
}

int readCO2()
{
  int co2_value = 0;  // We will store the CO2 value inside this variable.

  digitalWrite(13, HIGH);  // turn on LED
  // On most Arduino platforms this pin is used as an indicator light.

  //////////////////////////
  /* Begin Write Sequence */
  //////////////////////////

  Wire.beginTransmission(co2Addr);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x08);
  Wire.write(0x2A);

  Wire.endTransmission();

  /////////////////////////
  /* End Write Sequence. */
  /////////////////////////

  /*
    We wait 10ms for the sensor to process our command.
    The sensors's primary duties are to accurately
    measure CO2 values. Waiting 10ms will ensure the
    data is properly written to RAM

  */

  delay(10);

  /////////////////////////
  /* Begin Read Sequence */
  /////////////////////////

  /*
    Since we requested 2 bytes from the sensor we must
    read in 4 bytes. This includes the payload, checksum,
    and command status byte.

  */

  Wire.requestFrom(co2Addr, 4);

  byte i = 0;
  byte buffer[4] = {0, 0, 0, 0};

  /*
    Wire.available() is not nessessary. Implementation is obscure but we leave
    it in here for portability and to future proof our code
  */
  while (Wire.available())
  {
    buffer[i] = Wire.read();
    i++;
  }

  ///////////////////////
  /* End Read Sequence */
  ///////////////////////

  /*
    Using some bitwise manipulation we will shift our buffer
    into an integer for general consumption
  */

  co2_value = 0;
  co2_value |= buffer[1] & 0xFF;
  co2_value = co2_value << 8;
  co2_value |= buffer[2] & 0xFF;


  byte sum = 0; //Checksum Byte
  sum = buffer[0] + buffer[1] + buffer[2]; //Byte addition utilizes overflow

  if (sum == buffer[3])
  {
    // Success!
    digitalWrite(13, LOW);
    return co2_value;
  }
  else
  {
    // Failure!
    /*
      Checksum failure can be due to a number of factors,
      fuzzy electrons, sensor busy, etc.
    */

    digitalWrite(13, LOW);
    return 0;
  }
}

