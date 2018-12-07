
/*  
 * ------------------------------------------------------------------------
 * "PH2LB LICENSE" (Revision 1) : (based on "THE BEER-WARE LICENSE" Rev 42) 
 * <lex@ph2lb.nl> wrote this file. As long as you retain this notice
 * you can do modify it as you please. It's Free for non commercial usage 
 * and education and if we meet some day, and you think this stuff is 
 * worth it, you can buy me a beer in return
 * Lex Bolkesteijn 
 * ------------------------------------------------------------------------ 
 * Filename : qrpswr.ino  
 * Version  : 0.1 (DRAFT)
 * ------------------------------------------------------------------------
 * Description : simple qrp swr meter wit oled display based on 
 *  the digital QRP SWR meter design from Z2LPD 
 *  https://www.zl2pd.com/SWRmeter.html
 * ------------------------------------------------------------------------
 * Revision : 
 *  - 2018-dec-05 0.1 initial version 
 * ------------------------------------------------------------------------
 * Hardware used : 
 *  - Arduino Pro Mini
 *  - 128x64 OLED display
 *  - Breune directional coupler 
 * ------------------------------------------------------------------------
 * Software used : 
 *  - Adafruit_SSD1306 library  
 * ------------------------------------------------------------------------ 
 * TODO LIST : 
 *  - add more sourcode comment
 * ------------------------------------------------------------------------ 
 */
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_SSD1306.h>

#define SSD1306_LCDHEIGHT 64
 
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
 
#define HIGHSWRLED 8 

void setup()   {
  Serial.begin(9600);

  pinMode(HIGHSWRLED, OUTPUT);

  digitalWrite(HIGHSWRLED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(HIGHSWRLED, LOW);    // turn the LED off by making the voltage LOW
  delay(200);                       // wait for a second +
  digitalWrite(HIGHSWRLED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(HIGHSWRLED, LOW);    // turn the LED off by making the voltage LOW
  delay(200);                       // wait for a second

 
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done

  // Clear the buffer.
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("QRP power/swr meter");

  // display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.print("test : ");
  display.println(3.141592);
  //  display.setTextSize(2);
  //  display.setTextColor(WHITE);
  //  display.print("0x");
  //  display.println(0xDEADBEEF, HEX);
  display.display();
}

int counter = 0;

void updateDisplay(double pForward, double pReflected, long swr)
{ 
  // Clear the buffer.
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("QRP power/swr meter");
  display.print("FWD : ");
  display.print(pForward);
  display.println("W");
  display.print("REF : ");
  display.print(pReflected);
  display.println("W");
  display.print("SWR : ");
  if (swr >= 10)
  {
    display.print(swr / 10);
    display.print(".");
    display.print(swr % 10);
    display.println(":1");
  }
  else
  {
    display.println("No signal");
  }

  display.display(); 
}


void updateLog(double pForward, double pReflected,  long swr, long forward, long reflected)
{ 
  Serial.print("FWD : ");
  Serial.print(pForward);
  Serial.print("W (");
  Serial.print(forward);
  Serial.println(")");
  Serial.print("REF : ");
  Serial.print(pReflected);
  Serial.print("W (");
  Serial.print(reflected);
  Serial.println(")");
  Serial.print("SWR : ");
  if (swr >= 10)
  {
    Serial.print(swr / 10);
    Serial.print(".");
    Serial.print(swr % 10);
    Serial.println(":1");
  }
  else
  {
    Serial.println("No signal");
  }
}


void updateCSV(double pForward, double pReflected,  long swr, long forward, long reflected)
{ 
   
  Serial.print(pForward);
  Serial.print("|");
  Serial.print(forward);
  Serial.print("|");
  Serial.print(pReflected);
  Serial.print("|");
  Serial.print(reflected);
  Serial.print("|");
  if (swr >= 10)
  {
    Serial.print(swr / 10);
    Serial.print(".");
    Serial.print(swr % 10);
    Serial.println(":1");
  }
  else
  {
    Serial.println("No signal");
  }
  
}

void loop()
{

  long forward = analogRead(0);     // read the input pin
  long reflected = analogRead(1);     // read the input pin

  long swr = forward + reflected;
  swr = swr * 10;
  long temp = forward - reflected;
  if (forward <= reflected)
    temp = 1;

  swr = swr / temp;
  if (swr > 100)
    swr = 100;

  if (swr > 20)
    digitalWrite(HIGHSWRLED, HIGH);   // turn the LED on (HIGH is the voltage level)
  else
    digitalWrite(HIGHSWRLED, LOW);    // turn the LED off by making the voltage LOW
 
  double pForward = ((double)forward / 1024.0 * 5.0); // make it voltage
  pForward = (pForward * 7.07 * 0.707);
  pForward = pForward * pForward ;
  pForward = pForward / 50;

  // if there is a signal compensate for measurement error 
  // used a FT817 (measured the output with a quality digital power meter)
  // and made a compensation function. This may be need to be ajusted 
  // based on your implementation of the Breune directional coupler 
  if (pForward > 0.01)
    pForward -= (2.0 * pForward)/15.0 - (2.0/15.0);

  double pReflected = ((double)reflected / 1024.0 * 5.0); // make it voltage
  pReflected = (pReflected * 7.07 * 0.707);
  pReflected = pReflected * pReflected ;
  pReflected = pReflected / 50;

  // again signal compensation
  if (pReflected > 0.01)
    pReflected -= (2.0 * pReflected)/15.0 - (2.0/15.0);

  updateDisplay(pForward, pReflected, swr);
  //updateLog(pForward, pReflected, swr, forward, reflected);
  //updateCSV(pForward, pReflected, swr, forward, reflected);
  delay(100); 
}

