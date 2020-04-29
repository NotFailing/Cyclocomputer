//Including the nessescary ilbraries
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

//Used to keep track of what the TFT is displaying
int currentframe;

//Making the code compatible with specifically the Adafruit 480x320 Featheriwng screen
#define STMPE_CS 32
#define TFT_CS   15
#define TFT_DC   33
#define SD_CS    14
#define TFT_RST -1

//More avalible colours for the screen
#define HX8357_BLACK       0x0000  ///<   0,   0,   0
#define HX8357_NAVY        0x000F  ///<   0,   0, 123
#define HX8357_DARKGREEN   0x03E0  ///<   0, 125,   0
#define HX8357_DARKCYAN    0x03EF  ///<   0, 125, 123
#define HX8357_MAROON      0x7800  ///< 123,   0,   0
#define HX8357_PURPLE      0x780F  ///< 123,   0, 123
#define HX8357_OLIVE       0x7BE0  ///< 123, 125,   0
#define HX8357_LIGHTGREY   0xC618  ///< 198, 195, 198
#define HX8357_DARKGREY    0x7BEF  ///< 123, 125, 123
#define HX8357_BLUE        0x001F  ///<   0,   0, 255
#define HX8357_GREEN       0x07E0  ///<   0, 255,   0
#define HX8357_CYAN        0x07FF  ///<   0, 255, 255
#define HX8357_RED         0xF800  ///< 255,   0,   0
#define HX8357_MAGENTA     0xF81F  ///< 255,   0, 255
#define HX8357_YELLOW      0xFFE0  ///< 255, 255,   0
#define HX8357_WHITE       0xFFFF  ///< 255, 255, 255
#define HX8357_ORANGE      0xFD20  ///< 255, 165,   0
#define HX8357_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define HX8357_PINK        0xFC18  ///< 255, 130, 198

//Variables INA219 uses
float shuntvoltage;
float busvoltage;
float current_mA;
float loadvoltage;
float power_mW;  



#include <Adafruit_GPS.h>
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);

#define GPSECHO false

Adafruit_INA219 ina219;

// Init screen on hardware SPI, HX8357D type:
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

//Placeholder for the battery symbol
int percentage = 100;


void setup() {
  Serial.begin(115200);
  
  uint32_t currentFrequency;

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  GPSSerial.println(PMTK_Q_RELEASE);

  ina219.begin();

    // if the touchscreen returns true, assume we have the  tft device
    if (!ts.begin())
        return;
        
    //Setup for the TFT
    Serial.println("Touchscreen started");
     tft.begin();
     tft.setFont(&FreeSerif9pt7b);
     tft.setTextColor(HX8357_DARKCYAN);
     currentframe = 0;
     frame0();
}

//Defines the menu screen
void frame0() {

  tft.fillScreen(HX8357_BLACK);
  tft.setTextSize(2);

  tft.setCursor(90,30);
  
    if (GPS.hour < 10) { tft.print('0'); }
    tft.print(GPS.hour, DEC); tft.print(':');
    if (GPS.minute < 10) { tft.print('0'); }
    tft.print(GPS.minute, DEC); tft.print(':');
    if (GPS.seconds < 10) { tft.print('0'); }
    tft.print(GPS.seconds, DEC); 
    

  
  tft.setTextSize(2);
  tft.fillRoundRect(50, 150, 220, 100, 10, HX8357_ORANGE);
  tft.setCursor(80,225);
  tft.print("BATTERY");
  tft.fillRoundRect(50, 300, 220, 100, 10, HX8357_RED);
  tft.setCursor(100,370);
  tft.print("SPEED");
   }

//Defines the menutton which brigns you back to the menu screen
void returnbtn() {
  tft.fillRect(0, 405, 320, 75, HX8357_NAVY);
  tft.setCursor(90,455);
  tft.setTextSize(2);
  tft.print("RETURN");
}

//Defines the screen that is used to disply battery level
void frame1() {

  tft.fillScreen(HX8357_BLACK);
  tft.setCursor(0,25);
  tft.setTextSize(1);

  tft.print("Bus Voltage:   "); tft.print(busvoltage); tft.println(" V");
  tft.print("ShuntVoltage:"); tft.print(shuntvoltage); tft.println("mV");
  tft.print("Load Voltage:  "); tft.print(loadvoltage); tft.println("V");
  tft.print("Current:       "); tft.print(current_mA); tft.println(" mA");
  tft.print("Power:        "); tft.print(power_mW); tft.println("mW");
  tft.println("");

  tft.fillRoundRect(130, 80, 40, 20, 5, HX8357_WHITE);
  
  tft.fillRect(70, 100, 150, 20, HX8357_WHITE);
  tft.fillRect(70, 100, 20, 250, HX8357_WHITE);
  tft.fillRect(220, 100, 20, 270, HX8357_WHITE);
  tft.fillRect(70, 350, 150, 20, HX8357_WHITE);

  tft.fillRect(90, 120, 130, 80, HX8357_GREEN); 
  tft.fillRect(90, 200, 130, 80, HX8357_YELLOW);
  tft.fillRect(90, 280, 130, 80, HX8357_RED);

  
  tft.setTextSize(2);
  tft.setCursor(120,250);
  tft.print("100%");

  returnbtn();
}

//Defines the screen that is used to show the GPS' information
void frame2() {

 tft.fillScreen(HX8357_BLACK);
 tft.setCursor(0,50);  
 tft.setTextSize(2);
  
    tft.print("Fix: "); tft.println((int)GPS.fix);
    tft.print("Quality: "); tft.println((int)GPS.fixquality);
    if (GPS.fix) {
      tft.print("Location: ");
      tft.print(GPS.latitude, 4); tft.print(GPS.lat);
      tft.print(", ");
      tft.print(GPS.longitude, 4); tft.println(GPS.lon);
      tft.print("Speed (knots): "); tft.println(GPS.speed);
      tft.print("Angle: "); tft.println(GPS.angle);
      tft.print("Altitude: "); tft.println(GPS.altitude);
      tft.print("Satellites: "); tft.println((int)GPS.satellites);
    }

   returnbtn();
  }

void loop() {

//Enables the touch function of the TFT
TS_Point p = ts.getPoint();
p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

 //Used to monitor the coordinates in use 
 Serial.print("X = "); 
 Serial.print(p.x); 
 Serial.print("\tY = "); 
 Serial.print(p.y);  
 Serial.print("\tPressure = "); 
 Serial.println(p.z); 

 
 //The diffeent values the INA reads
 shuntvoltage = ina219.getShuntVoltage_mV();
 busvoltage = ina219.getBusVoltage_V();
 current_mA = ina219.getCurrent_mA();
 power_mW = ina219.getPower_mW();
 loadvoltage = busvoltage + (shuntvoltage / 1000);

 char c = GPS.read();
    
 //Sets the cooridantes whitin which the battery button work
 if( 150 < p.y && p.y < 250){
     if (50 < p.x && p.x< 270){
      if(currentframe == 0){
        currentframe = 1;

        frame1();
      }
     } 
    }

  
  //Sets the cooridantes whitin which the speed button work
  if(300 < p.y && p.y < 400){
   if(50 < p.x && p.x < 270){
    if (currentframe == 0){
      currentframe = 2;

      frame2();
    }
  }
 }

 //Sets the cooridantes whitin which the return button works
 if(405 < p.y && p.y < 480){
  if(0 < p.x && p.x < 320){
    if (currentframe == 1 or currentframe == 2){
      currentframe = 0;

      frame0();
    }
  }
 }
}
