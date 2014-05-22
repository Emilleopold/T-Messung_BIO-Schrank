/*
T-Messung im BIO-Schrank
Verwendet :
 LiquidCrystal Library
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
  The circuit:
 * LCD RS pin to digital pin 12 (DB8)
 * LCD Enable pin to digital pin 11 (D9)
 * LCD D4 pin to digital pin 5 (D4)
 * LCD D5 pin to digital pin 4 (D5)
 * LCD D6 pin to digital pin 3 (D6)
 * LCD D7 pin to digital pin 2 (D7)
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 This example code is in the public domain.
 http://arduino.cc/en/Tutorial/LiquidCrystalScroll


  Board	          int.0	  int.1	  int.2	  int.3	  int.4	  int.5
 Uno, Ethernet	  2	  3
 Mega2560	  2	  3	  21	  20	  19	  18
 Leonardo	  3	  2	  0	  1
 Due	          (any pin, more info http://arduino.cc/en/Reference/AttachInterrupt)
 */

// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//
double dewPoint(double celsius, double humidity)
{
	// (1) Saturation Vapor Pressure = ESGG(T)
	double RATIO = 373.15 / (273.15 + celsius);
	double RHS = -7.90298 * (RATIO - 1);
	RHS += 5.02808 * log10(RATIO);
	RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
	RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
	RHS += log10(1013.246);

        // factor -3 is to adjust units - Vapor Pressure SVP * humidity
	double VP = pow(10, RHS - 3) * humidity;

        // (2) DEWPOINT = F(Vapor Pressure)
	double T = log(VP/0.61078);   // temp var
	return (241.88 * T) / (17.558 - T);
}

// delta max = 0.6544 wrt dewPoint()
// 6.9 x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
	double a = 17.271;
	double b = 237.7;
	double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
	double Td = (b * temp) / (a - temp);
	return Td;
}


// include the library code:
#include <dht11.h>
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);   // select the pins used on the LCD panel

dht11 DHT11;
#define DHT11PIN 2

  float TempOld = 0.0;
  float TempAct = 0.0;
  float TempMax = 0.0;
  float TempMin = 999.0;
  float HumiOld = 0.0; 
  float HumiAct = 0.0;
  float HumiMax = 0.0;
  float HumiMin = 999.0;
  long Counter = 0;
  boolean Toggle = false;


void setup() {
  Serial.begin(9600);
  Serial.println("DHT11 TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.clear();
  delay(1000);
}

void loop() {
  lcd.clear();
  Serial.println("\n");
  int chk = DHT11.read(DHT11PIN);
  Serial.print("Read sensor: ");
  lcd.setCursor(14,0);
  switch (chk)
  {
    case DHTLIB_OK: 
		Serial.println("OK");
                lcd.print("OK");
		break;
    case DHTLIB_ERROR_CHECKSUM: 
		Serial.println("Checksum error"); 
                lcd.print("CS");
		break;
    case DHTLIB_ERROR_TIMEOUT: 
		Serial.println("Time out error"); 
                lcd.print("TO");
		break;
    default: 
		Serial.println("Unknown error"); 
                lcd.print("UE");
		break;
  }
  lcd.setCursor(0,0);
  lcd.print("CNT: ");
  lcd.print(Counter);
  ++Counter;

  TempAct = DHT11.temperature;
  HumiAct = DHT11.humidity;
  if (TempAct > TempMax) TempMax = TempAct;
  if (TempAct < TempMin) TempMin = TempAct;
  if (HumiAct > HumiMax) HumiMax = HumiAct;
  if (HumiAct < HumiMin) HumiMin = HumiAct;
  
// /*  
  if (Toggle == true) {
    lcd.setCursor(0,1);
    lcd.print("T:");
    lcd.print((int)TempAct);
    lcd.setCursor(4,1);
    lcd.print(" H:");
    lcd.print((int)TempMax);
    lcd.setCursor(10,1);
    lcd.print(" L:");
    lcd.print((int)TempMin);
    Toggle = false;
    }
  else {
    lcd.setCursor(0,1);
    lcd.print("H:");
    lcd.print((int)HumiAct);
    lcd.setCursor(4,1);
    lcd.print(" H:");
    lcd.print((int)HumiMax);
    lcd.setCursor(10,1);
    lcd.print(" L:");
    lcd.print((int)HumiMin);
    Toggle = true;
    }
// */
  
  Serial.print("Humidity (%): ");
  Serial.print((float)HumiAct, 2);
  Serial.print(" Humidity Max (%): ");
  Serial.print((float)HumiMax, 2);
  Serial.print(" Humidity Min (%): ");
  Serial.println((float)HumiMin, 2);

  Serial.print("Temperature (°C): ");
  Serial.print((float)TempAct, 2);
  Serial.print(" Temperature Max (°C): ");
  Serial.print((float)TempMax, 2);
  Serial.print(" Temperature Min (°C): ");
  Serial.println((float)TempMin, 2);

  Serial.print("Dew Point (°C): ");
  Serial.println(dewPoint(TempAct, HumiAct));

  Serial.print("Dew PointFast (°C): ");
  Serial.println(dewPointFast(TempAct, HumiAct));

  // delay at the end of the full loop:
  delay(2000);

}

