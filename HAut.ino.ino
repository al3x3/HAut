/*

  Menu.pde

  Simple Menu Selection


  Universal 8bit Graphics Library, https://github.com/olikraus/u8glib/

*/
#include "U8glib.h"
#include <dht11.h>
#include <stdlib.h>
//***********************************************************
// define Nokia 5110 PCD8544 pins to arduino
#define PIN_Nok5110_RST    8
#define PIN_Nok5110_CE    10
#define PIN_Nok5110_DC     9
#define PIN_Nok5110_DIN   11
#define PIN_Nok5110_CLK   13

// define DHT11 pin to arduino
#define PIN_DHT11          2

// define Photocell pin to arduino
#define PIN_Photocell      0

// sedine Key pins to arduino
#define PIN_KEY_PREV       1
#define PIN_KEY_NEXT       3
#define PIN_KEY_SELECT     7
#define PIN_KEY_BACK       4

// output
// define LED1 pin to arduino
#define PIN_LED1          5   // PWM
// define LED2 pin to arduino
#define PIN_LED2          0
// define RELAY1 pin to arduino
#define PIN_RELAY1        0
// define RELAY2 pin to arduino
#define PIN_RELAY2        0
//***********************************************************


#define KEY_NONE 0
#define KEY_PREV 1
#define KEY_NEXT 2
#define KEY_SELECT 3
#define KEY_BACK 4

uint8_t uiKeyPrev = PIN_KEY_PREV;
uint8_t uiKeyNext = PIN_KEY_NEXT;
uint8_t uiKeySelect = PIN_KEY_SELECT;
uint8_t uiKeyBack = PIN_KEY_BACK;

uint8_t uiKeyCodeFirst = KEY_NONE;
uint8_t uiKeyCodeSecond = KEY_NONE;
uint8_t uiKeyCode = KEY_NONE;

dht11 DHT11;


//U8GLIB_PCD8544 u8g(13, 11, 10, 9, 8);
U8GLIB_PCD8544 u8g(PIN_Nok5110_CLK, PIN_Nok5110_DIN, PIN_Nok5110_CE, PIN_Nok5110_DC, PIN_Nok5110_RST);
const unsigned char  logo16_glcd_bmp[] =
{ B00000001, B10000000,
  B00000011, B11000000,
  B00000111, B11100000,
  B00001111, B11110000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000
};

void uiSetup(void) {
  // configure input keys
  pinMode(uiKeyPrev, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeyNext, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeySelect, INPUT_PULLUP);         // set pin to input with pullup
  pinMode(uiKeyBack, INPUT_PULLUP);           // set pin to input with pullup
}

void uiStep(void) {
  uiKeyCodeSecond = uiKeyCodeFirst;
  if ( digitalRead(uiKeyPrev) == LOW )
    uiKeyCodeFirst = KEY_PREV;
  else if ( digitalRead(uiKeyNext) == LOW )
    uiKeyCodeFirst = KEY_NEXT;
  else if ( digitalRead(uiKeySelect) == LOW )
    uiKeyCodeFirst = KEY_SELECT;
  else if ( digitalRead(uiKeyBack) == LOW )
    uiKeyCodeFirst = KEY_BACK;
  else
    uiKeyCodeFirst = KEY_NONE;

  if ( uiKeyCodeSecond == uiKeyCodeFirst )
    uiKeyCode = uiKeyCodeFirst;
  else
    uiKeyCode = KEY_NONE;
}


#define MENU_ITEMS 4
#define SUB_MENU_ITEMS_TEMP 3
const char *menu_strings[MENU_ITEMS] = { "Temperature", "Light Sensor", "PIR Sensor", "Smoke Sensor" };
//const char *sub_menu_strings[MENU_ITEMS][SUB_MENU_ITEMS_TEMP] = {{ "T(°C): ", "H(%): ", "DewP(°C): " },{"Light : ",NULL,""},{"Movement : ","",""},{"None","",""}};

double temp_vals[3];
float light_vals;
char light_strings[15];

#define MENU_TEMP_UMD   0
#define MENU_LGHT       1
#define MENU_PIR        2
#define MENU_MET        3

uint8_t menu_current = 0;
uint8_t menu_redraw_required = 0;
uint8_t last_key_code = KEY_NONE;
uint8_t read_counter = 0;
int8_t menu_call = -1;

void drawMenu(void) {
  uint8_t i, h;
  u8g_uint_t w, d;

  //u8g.setFont(u8g_font_6x13);u8g_font_5x7
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();

  h = u8g.getFontAscent() - u8g.getFontDescent();
  w = u8g.getWidth();
  for ( i = 0; i < MENU_ITEMS; i++ ) {
    d = (w - u8g.getStrWidth(menu_strings[i])) / 2;
    u8g.setDefaultForegroundColor();
    if ( i == menu_current ) {
      u8g.drawBox(0, i * h + 1, w, h);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i * h, menu_strings[i]);
  }
}
//temp_strings
void drawMenuCall(uint8_t menu_to_display) {
  uint8_t i, h;
  u8g_uint_t w, d;
  char tempstr[15];
  char tempstr2[15];
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  u8g.setDefaultForegroundColor();
  h = u8g.getFontAscent() - u8g.getFontDescent();
  w = u8g.getWidth();
  switch (menu_to_display)
  {
    case MENU_TEMP_UMD:
      dtostrf(temp_vals[0], 2, 0, tempstr);
      sprintf(tempstr2, "Temp  : %s C", tempstr);
      u8g.drawStr(1, 1 * h, tempstr2);
      dtostrf(temp_vals[1], 2, 0, tempstr);
      sprintf(tempstr2, "Humid : %s %%", tempstr);
      u8g.drawStr(1, 2 * h, tempstr2);
      dtostrf(temp_vals[2], 2, 0, tempstr);
      sprintf(tempstr2, "DewPt : %s C", tempstr);
      u8g.drawStr(1, 3 * h, tempstr2);
      break;
    case MENU_LGHT:
      dtostrf(light_vals, 0, 0, tempstr);
      //sprintf(tempstr2, "Light:%s lux", tempstr);
      //u8g.drawStr(1, 1 * h, tempstr2);
      d = (w - u8g.getStrWidth("Light")) / 2;
      u8g.drawStr(d, 0 * h, "Light");
      u8g.drawHLine(0, 13, 84);
      sprintf(tempstr2, "%s lux", tempstr);
      d = (w - u8g.getStrWidth(tempstr2)) / 2;
      u8g.drawStr(d, 2 * h, tempstr2);
      d = (w - u8g.getStrWidth(light_strings)) / 2;
      u8g.drawStr(d, 3 * h, light_strings);
      break;
    default:
      d = (w - u8g.getStrWidth("ERROR")) / 2;
      u8g.drawStr(d, 2 * h, "ERROR");
    break;  
  }

}


void updateMenu(void) {
  if ( uiKeyCode != KEY_NONE && last_key_code == uiKeyCode ) {
    return;
  }
  last_key_code = uiKeyCode;

  switch ( uiKeyCode ) {
    case KEY_NEXT:
      menu_current++;
      if ( menu_current >= MENU_ITEMS )
        menu_current = 0;
      menu_redraw_required = 1;
      break;
    case KEY_PREV:
      if ( menu_current == 0 )
        menu_current = MENU_ITEMS;
      menu_current--;
      menu_redraw_required = 1;
      break;
    case KEY_SELECT:
      menu_call = menu_current;
      menu_redraw_required = 1;
      break;
    case KEY_BACK:
      menu_call = -1;
      menu_current = 0;
      menu_redraw_required = 1;
      break;
  }
}

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
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);

  // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10, RHS - 3) * humidity;

  // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP / 0.61078); // temp var
  return (241.88 * T) / (17.558 - T);
}

void read_DHT11(void) {
  Serial.println("\n");
  int chk = DHT11.read(PIN_DHT11);
  char tempstr[25];

  Serial.print("Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK:
      Serial.println("OK");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.println("Checksum error");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.println("Time out error");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }

  Serial.print("Temperature (deg C): ");
  //Serial.println((float)DHT11.temperature,0);
  //sub_menu_results[MENU_TEMP_UMD][0]=(float)DHT11.temperature;
  temp_vals[0] = (float)DHT11.temperature;
  dtostrf(temp_vals[0], 2, 0, tempstr);
  //sprintf(sub_menu_results[MENU_TEMP_UMD][0], "%f", (float)DHT11.temperature);
  Serial.println(tempstr);

  Serial.print("Humidity (%): ");

  //Serial.println((float)DHT11.humidity,0);
  temp_vals[1] = (float)DHT11.humidity;
  dtostrf(temp_vals[1], 2, 0, tempstr);
  //sub_menu_results[MENU_TEMP_UMD][1]=(float)DHT11.humidity;
  //sprintf(sub_menu_results[MENU_TEMP_UMD][1], "%0.2f", (float)DHT11.humidity);
  Serial.println(tempstr);
  Serial.print("Dew Point (deg C): ");
  temp_vals[2] = dewPoint(DHT11.temperature, DHT11.humidity);
  dtostrf(temp_vals[2], 2, 0, tempstr);
  //Serial.println(dewPoint(DHT11.temperature, DHT11.humidity),0);
  //sub_menu_results[MENU_TEMP_UMD][2]=dewPoint(DHT11.temperature, DHT11.humidity);
  //sprintf(sub_menu_results[MENU_TEMP_UMD][2], "%0.2f", dewPoint(DHT11.temperature, DHT11.humidity));
  Serial.println(tempstr);
}


void read_Photocell(void) {
  int photocellReading;
  int LEDbrightness;
  float measVolt=0;
  char tempstr[25];
  //Serial.println("\n");
  photocellReading = analogRead(PIN_Photocell);
  //light_vals = photocellReading;
  measVolt = (double)(5.0 * photocellReading ) /1024.0;
  
  //int light_vals;
  //char light_strings[15];
  Serial.print("Voltage Analog reading = ");
  Serial.println(measVolt,2);
  Serial.print("Raw Analog reading = ");
  
  Serial.print(photocellReading);     // the raw analog reading

  // We'll have a few threshholds, qualitatively determined
  if (photocellReading < 10) {
    sprintf(light_strings, "%s", " - Dark - ");
    //Serial.println(" - Dark");
  } else if (photocellReading < 200) {
    sprintf(light_strings, "%s", " - Dim - ");
    //Serial.println(" - Dim");
  } else if (photocellReading < 500) {
    sprintf(light_strings, "%s", " - Light - ");
    //Serial.println(" - Light");
  } else if (photocellReading < 800) {
    sprintf(light_strings, "%s", " - Bright - ");
    //Serial.println(" - Bright");
  } else {
    sprintf(light_strings, "%s", "-Very bright-");
  }
  Serial.println(light_strings);
  // linear interpolation for 10K resistor https://learn.adafruit.com/photocells/using-a-photocell
  float out[] = {1.0,10.0,100.0,1000.0,10000.0};
  float in[]  = {0.1,0.5,2.0,3.8,4.5};
  light_vals = FmultiMap(measVolt, in, out, 5);
  Serial.print("Ambient light (lux) = ");
  Serial.println(light_vals,2);
  


  // LED gets brighter the darker it is at the sensor
  // that means we have to -invert- the reading from 0-1023 back to 1023-0
  photocellReading = 1023 - photocellReading;
  //now we have to map 0-1023 to 0-255 since thats the range analogWrite uses
  LEDbrightness = map(photocellReading, 0, 1023, 0, 255);
  analogWrite(PIN_LED1, LEDbrightness);
}

void setup() {
  // serial setup
  Serial.begin(115200);
  Serial.println("DHT11 TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();

  // rotate screen, if required
  // u8g.setRot180();

  uiSetup();                    // setup key detection and debounce algorithm
  menu_redraw_required = 1;     // force initial redraw


}

void loop() {
  // read sensors data
  read_counter++;
  if (read_counter > 20)
  {
    read_DHT11();
    read_Photocell();
    read_counter = 0;
    if (menu_call != -1) {
      menu_redraw_required = 1;
    }
  }
  // check buttons state
  uiStep(); // check for key press

  if (  menu_redraw_required != 0 ) {
    u8g.firstPage();
    do  {
      if (menu_call == -1) {
        drawMenu();
      }
      else {
        drawMenuCall(menu_call);
      }
    } while ( u8g.nextPage() );
    menu_redraw_required = 0;
  }

  updateMenu();                            // update menu bar
  delay(100);

}

 // note: the in array should have increasing values
float FmultiMap(float val, float * _in, float * _out, uint8_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}
