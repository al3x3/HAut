/*

  Menu.pde
  
  Simple Menu Selection

  
  Universal 8bit Graphics Library, https://github.com/olikraus/u8glib/
  
*/
#include "U8glib.h"
#include <dht11.h>
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
#define PIN_KEY_PREV       3
#define PIN_KEY_NEXT       1
#define PIN_KEY_SELECT     7
#define PIN_KEY_BACK       4

// output
// define LED1 pin to arduino
#define PIN_LED1          6   // PWM
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
const char *menu_strings[MENU_ITEMS] = { "Temperature & Humidity", "Light Sensor", "PIR Sensor", "Metan Sensor" };
const char *sub_menu_strings[MENU_ITEMS][SUB_MENU_ITEMS_TEMP] = {{ "Temperature (°C): ", "Humidity (%): ", "Dew Point (°C): " },{"Light : ",NULL,""},{"Movement : ","",""},{"None","",""}};
char * sub_menu_results[MENU_ITEMS][SUB_MENU_ITEMS_TEMP];

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

  u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  
  h = u8g.getFontAscent()-u8g.getFontDescent();
  w = u8g.getWidth();
  for( i = 0; i < MENU_ITEMS; i++ ) {
    d = (w-u8g.getStrWidth(menu_strings[i]))/2;
    u8g.setDefaultForegroundColor();
    if ( i == menu_current ) {
      u8g.drawBox(0, i*h+1, w, h);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i*h, menu_strings[i]);
  }
}

void drawMenuCall(uint8_t menu_to_display) {
  uint8_t i, h;
  u8g_uint_t w, d;
  char tempstr[25]; // enough to hold all numbers up to 64-bits
  
  u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  u8g.setDefaultForegroundColor();
  
  h = u8g.getFontAscent()-u8g.getFontDescent();
  w = u8g.getWidth();
  for( i = 0; i < SUB_MENU_ITEMS_TEMP; i++ ) {
    if (sub_menu_strings[menu_to_display][i] != NULL)
    {
      sprintf(tempstr, "%s %s", sub_menu_strings[menu_to_display][i],sub_menu_results[menu_to_display][i]);
      d = (w-u8g.getStrWidth(tempstr))/2;
      u8g.drawStr(d, i*h, tempstr);
    }
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
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);

        // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10, RHS - 3) * humidity;

        // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP/0.61078);   // temp var
  return (241.88 * T) / (17.558 - T);
}

void read_DHT11(void){
    Serial.println("\n");
    int chk = DHT11.read(PIN_DHT11);

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

  Serial.print("Temperature (°C): ");
  //sub_menu_results[MENU_TEMP_UMD][0]=(float)DHT11.temperature;  
  sprintf(sub_menu_results[MENU_TEMP_UMD][0], "%0.2f", (float)DHT11.temperature);
  Serial.println(sub_menu_results[MENU_TEMP_UMD][0]);
  Serial.print("Humidity (%): ");
  //sub_menu_results[MENU_TEMP_UMD][1]=(float)DHT11.humidity;
  sprintf(sub_menu_results[MENU_TEMP_UMD][1], "%0.2f", (float)DHT11.humidity);
  Serial.println(sub_menu_results[MENU_TEMP_UMD][1]);
  Serial.print("Dew Point (°C): ");
  //sub_menu_results[MENU_TEMP_UMD][2]=dewPoint(DHT11.temperature, DHT11.humidity);  
  sprintf(sub_menu_results[MENU_TEMP_UMD][2], "%0.2f", dewPoint(DHT11.temperature, DHT11.humidity));
  Serial.println(sub_menu_results[MENU_TEMP_UMD][2]);

}


void read_Photocell(void){
  int photocellReading;
  int LEDbrightness;  
  Serial.println("\n");
  photocellReading = analogRead(PIN_Photocell);  
 
  Serial.print("Analog reading = ");
  Serial.print(photocellReading);     // the raw analog reading
 
  // We'll have a few threshholds, qualitatively determined
  if (photocellReading < 10) {
    sprintf(sub_menu_results[MENU_LGHT][0], " %s", " - Dark");
    Serial.println(" - Dark");
  } else if (photocellReading < 200) {
    sprintf(sub_menu_results[MENU_LGHT][0], " %s", " - Dim");
    Serial.println(" - Dim");
  } else if (photocellReading < 500) {
    sprintf(sub_menu_results[MENU_LGHT][0], " %s", " - Light");
    Serial.println(" - Light");
  } else if (photocellReading < 800) {
    sprintf(sub_menu_results[MENU_LGHT][0], " %s", " - Bright");
    Serial.println(" - Bright");
  } else {
    sprintf(sub_menu_results[MENU_LGHT][0], " %s", " - Very bright");
    Serial.println(" - Very bright");
  }
  
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
  // check temp&umidity sensor
  read_counter++;
  if (read_counter > 20)
  {
    read_DHT11();
    read_counter=0;
  }
  // check buttons state
  uiStep();                                     // check for key press
    
  if (  menu_redraw_required != 0 ) {
    u8g.firstPage();
    do  {
      if (menu_call == -1){
        drawMenu();
      }
      else {
        drawMenuCall(menu_call);
      }
    } while( u8g.nextPage() );
    menu_redraw_required = 0;
  }

  updateMenu();                            // update menu bar
  delay(100);
  
}
