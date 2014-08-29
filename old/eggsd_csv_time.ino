#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);        // HW SPI Com: CS = 10, A0 = 9 (Hardware Pins are  SCK = 13 and MOSI = 11
//-------字体设置，大、中、小
#define setFont_L u8g.setFont(u8g_font_7x13)
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)
#define setFont_S u8g.setFont(u8g_font_chikitar)

// SD SPICom: MOSI=11; MISO=12 ;SLK =13 ;CS=4
const int chipSelect=4;  


#include <iterator>
#include <string>
#include <pnew.cpp>

//=============================
#define INTERVAL_LCD             100
#define INTERVAL_LCD_SWITCH      2000
#define INTERVAL_SENSOR          500

//*60*2
unsigned long timer = millis();
unsigned long updata_time=millis();
unsigned long lcd_time=millis();
unsigned long lcd_switch_time=millis();
unsigned long sensor_time=millis();

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
Adafruit_GPS GPS(&Serial1);
#define GPSECHO false
boolean STA;                       
float f_latitude,f_longitude;        //经纬度
char c_lat,c_lon;                //经纬极向
int itime[3];        //时间
int idate[3];        //日期
float f_Speed;        //速度
int i_Speed[2];        //速度格式化
float f_Height;        //海拔
int i_satellites;        //卫星数
float f_fixquality;        //信号质量

#define init_updata 1000                        //gps数据刷新时间
#define init_oled 500                        //OLED刷新时间

unsigned long time_oled = millis();

//=============================
#include <Wire.h>

#include <AM2321.h>
AM2321 am2321;

#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);

#include <SD.h>
File myFile;

float sensor_tem,sensor_hum,sensor_lux,sensor_alt,sensor_pre;

void volcdsetup(char* zi,unsigned int x,unsigned int y)
{
  u8g.firstPage();  
  do {
    setFont_M;    
    u8g.setPrintPos(x, y); 
    u8g.print(zi);
  } 
  while( u8g.nextPage() );
}

void setup()
{ 
  Serial.begin(115200);     
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  GPS.begin(38400); 
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(10, OUTPUT);
   if (!SD.begin(7)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
   
  Serial.println(bmp.begin() ? "BMP085 connection successful" : "BMP085 connection failed");
  Serial.println(tsl.begin() ? "TSL2561 connection successful" : "TSL2561 connection failed");
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoGain(true);          /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  delay(500);
}

void vogps_dataread()
 {
   char c = GPS.read();
   // if you want to debug, this is a good time to do it!
   // if a sentence is received, we can check the checksum, parse it...
   if (GPS.newNMEAreceived()) {
     // a tricky thing here is if we print the NMEA sentence, or data
     // we end up not listening and catching other sentences!
     // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
     if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
       return; // we can fail to parse a sentence in which case we should just wait for another
   }
 
   if (timer > millis())
     timer = millis();
   if (millis() - timer > init_updata)
   {
     timer = millis(); // reset the timer
     if(GPS.hour>=0&&GPS.hour<=16)
     itime[0]=GPS.hour+8;
     else
     itime[0]=GPS.hour;
     itime[1]=GPS.minute;
     itime[2]=GPS.seconds;

     idate[0]=GPS.year;
     idate[1]=GPS.month;
     idate[2]=GPS.day;
     f_fixquality=GPS.fixquality;        //信号质量
     STA=GPS.fix;                        //GPS定位状态

    if (STA)                //当GPS定位上
    {
       f_latitude=GPS.latitude;
       f_longitude=GPS.longitude;
       c_lat=GPS.lat;
       c_lon=GPS.lon;

      f_Speed=1.852*GPS.speed;                        //速度转化
      i_Speed[0]=int(f_Speed*10)%10;        //速度格式化
      i_Speed[1]=int(f_Speed);                        //速度格式化

      f_Height=GPS.altitude;                        //海拔

      i_satellites=GPS.satellites;      //卫星数
    }
   }
 }

void loop(void)
{   
   vogps_dataread();
   
  if (sensor_time > millis()) sensor_time = millis();
  if(millis()-sensor_time>INTERVAL_SENSOR)
  {
    {
      am2321.read();
      sensor_tem=am2321.temperature/10.0;
      sensor_hum=am2321.humidity/10.0;
    }
    {
      sensor_alt=bmp.readAltitude(101500);
      sensor_pre=bmp.readPressure();
    }
    {
      sensors_event_t event;
      tsl.getEvent(&event);

      /* Display the results (light is measured in lux) */
      if (event.light)
        sensor_lux=event.light;
      else
        Serial.println("Sensor overload");
    }
    sensor_time=millis();
  }  
  
 {
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 6; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ","; 
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(idate[0]);
    dataFile.println(idate[1]);
    dataFile.println(idate[2]);
    dataFile.println(itime[0]);
    dataFile.println(itime[1]);
    dataFile.println(itime[2]);
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:   
    Serial.println(dataString);
    }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.csv");
   } 
  }


  if (lcd_time > millis()) lcd_time = millis();
  if(millis()-lcd_time>INTERVAL_LCD)
  {
    volcd();
    lcd_time=millis();
  }

}


 void volcd()
 {
  u8g.firstPage();  
  do {
    setFont_L;

    u8g.setPrintPos(0, 15*1); 
    u8g.print(sensor_tem ,1);
    u8g.print(" `C");
    u8g.print(" ");
    u8g.print(sensor_hum ,1);
    u8g.print(" %");
     /*
    u8g.setPrintPos(0, 15*2); 
    u8g.print(sensor_pre/1000.0 ,3);
    u8g.print(" kPa");
     */
   u8g.setPrintPos(0, 15*2);
   u8g.print("20");
   u8g.print(idate[0],2);
   u8g.print("-");
   u8g.print(idate[1],2);
   u8g.print("-");
   u8g.print(idate[2],2);
   
   u8g.setPrintPos(0, 15*3);
   u8g.print("  ");
   u8g.print(itime[0],1);
   u8g.print(":");
   u8g.print(itime[1],2);
   u8g.print(":");
   u8g.print(itime[2],2);

   /*
    u8g.setPrintPos(0, 15*5); 
    u8g.print(sensor_lux ,1);
    u8g.print(" Lux");
    */
  } 
  while( u8g.nextPage() );
}

