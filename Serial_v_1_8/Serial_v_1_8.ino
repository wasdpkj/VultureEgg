#define FROM Serial
#define SEND Serial
#define DEBUG Serial

//======================

byte  egg_mpuBuf[12];
/*
AA BB AA 1A 01 2A 01 3A 01 4A 01 5A 01 6A 01 FF FF
 */

byte  egg_temBuf[28];
/*
AA BB BB 1A 01 2A 01 3A 01 4A 01 5A 01 6A 01 7A 01 8A 01 9A 01 1B 02 2B 02 3B 02 4B 02 5B 02 FF FF
 */
byte  egg_humBuf[2];
/*
AA BB CC 1A 01 FF FF
 */

//-------------------------
unsigned long time1=millis();
int type;

#define egg_UUID "0017EA0943AE"

float egg_mpu[6];
float egg_tem[14];
float egg_hum[1];

//=============================
int p;

uint16_t read16(byte* _Buf) {
  uint16_t r = (_Buf[p++]&0xFF);
  r+= (_Buf[p++]&0xFF)<<8;
  return r;
}

void read_data(int _num,byte* _buf)
{
  p=0;
  _num=_num/2;
  //  inBuf=_buf;
  float _bufin[_num];
  for(int i=0;i<_num;i++) {
    //DEBUG.print("RC[");
    //DEBUG.print(i+1);
    //DEBUG.print("]:");

    //DEBUG.print(_buf[2*i],DEC);
    //DEBUG.print(",");
    //DEBUG.print(_buf[2*i+1],DEC);

    //DEBUG.print("   :");
    _bufin[i]=read16(_buf);
    //DEBUG.println(_bufin[i]);
    //delay(50);        // delay in between reads for stability
  }

  DEBUG.println("\n\r");

  switch(type)
  {
  case 1: 
    DEBUG.print("DATA_MPU: ");
    for(int a=0;a<_num;a++)
    {
      egg_mpu[a]=_bufin[a];
      DEBUG.print(egg_mpu[a]);
      DEBUG.print(",");
    }
    break;
  case 2: 
    DEBUG.print("DATA_TEM: ");
    for(int a=0;a<_num;a++)
    {
      egg_tem[a]=_bufin[a]/16.0;
      DEBUG.print(egg_tem[a]);
      if(a<_num-1)
        DEBUG.print(",");
    }
    break;
  case 3: 
    DEBUG.print("DATA_HUM: ");
    for(int a=0;a<_num;a++)
    {
      egg_hum[a]=_bufin[a];
      DEBUG.print(egg_hum[a]);
      DEBUG.print(",");
    }
    break;
  default:
    DEBUG.print("DATA_ERROR: ");
  }

  DEBUG.println(" ");
}

unsigned long num=0;

byte pkj[512];

byte inChar,inCache;

boolean sta =false;
boolean error=false;

//================
#include <Wire.h>
#include <Rtc_Pcf8563.h>

//init the real time clock
Rtc_Pcf8563 rtc;

#define SECONDS_IN_HOUR          3600
#define SECONDS_IN_DAY          86400
#define START_YEAR              1970
#define TIME_ZONE               8
static int days_in_month[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int year,month,day,hour,minute,second;

float TimeStamp;

void getRTC()
{
  //----------------------
  rtc.getDate();
  rtc.getTime();

  //----------------------
  year=rtc.getYear();
  month=rtc.getMonth();
  day=rtc.getDay();
  hour=rtc.getHour();
  minute=rtc.getMinute();
  second=rtc.getSecond();
}

uint32_t getTimeStamp(bool _getTimeStamp)
{
  //----------------------
  getRTC();

  //----------------------
  uint32_t _timeStamp=0;
  _timeStamp += second;
  _timeStamp += minute*60;
  _timeStamp += hour*SECONDS_IN_HOUR;
  _timeStamp += (day-1)*SECONDS_IN_DAY;

  int _month=1;
  while(1) 
  {    
    uint32_t seconds;
    if(isLeapYear(year) && _month == 2) 
      seconds = SECONDS_IN_DAY * 29;
    else
      seconds = SECONDS_IN_DAY * days_in_month[_month-1];

    if(_month < month) 
    {
      _timeStamp += seconds;
      _month++;
    } 
    else break;
  }  

  int _year=START_YEAR;
  while(1) 
  {
    uint32_t seconds;
    if(isLeapYear(_year)) 
      seconds = SECONDS_IN_DAY * 366;
    else
      seconds = SECONDS_IN_DAY * 365;
    if(_year<2000+year) 
    {
      _timeStamp += seconds;
      _year++;
    } 
    else break;
  }

  _timeStamp -=TIME_ZONE*SECONDS_IN_HOUR;

  //---------------
  if(_getTimeStamp)
  {
    DEBUG.print("20");
    DEBUG.print(year);
    DEBUG.print("/");
    DEBUG.print(month);
    DEBUG.print("/");
    DEBUG.print(day);
    DEBUG.print("  ");
    DEBUG.print(hour);
    DEBUG.print(":");
    DEBUG.print(minute);
    DEBUG.print(":");
    DEBUG.print(second);
    DEBUG.print("\n\r");
  }

  return _timeStamp;
}

boolean isLeapYear(unsigned int _year) 
{
  return (_year % 4 == 0 && (_year % 100 != 0 || _year % 400 == 0));
}

//================
#include <Wire.h>                                 //调用库  
#include "I2Cdev.h"                             //调用库  
//温度   
#include <OneWire.h>
OneWire ds(6);        //D6
//湿度   
#include <SHT2x.h>
//气压   
#include "BMP085.h"                             //调用库  
BMP085 barometer;   
//光照   
#include <Adafruit_TSL2561_U.h>                 //调用库  
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);   

//3,传感器值的设置  
float sensor_tem, sensor_hum,sensor_lux,sensor_pre,sensor_alt;    //温度 湿度 光照 气压 海拔

float getTem()
{
  //0.1C
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];
  if ( !ds.search(addr)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    return 0;
  }
  if ( OneWire::crc8( addr, 7) != addr[7]) {
    return 1000;
  }
  if ( addr[0] != 0x28) {
    return 1000;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end
  delay(50);
  ds.reset();
  ds.select(addr);  
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  ds.reset_search();
  float tempRead = (data[1] << 8) | data[0]; //using two's compliment
  return (float)tempRead/16.0;
}

float getHum()
{
  return SHT2x.GetHumidity();
}

float getLux()
{
  //获取光照===============================================    
  sensors_event_t event;   
  tsl.getEvent(&event);   
  if(event.light)
    return event.light;
  else
    return 0;   
}

float getPre()
{
  //获取气压、海拔========================================== 
  barometer.setControl(BMP085_MODE_TEMPERATURE);   
  unsigned long lastMicros = micros();      
  //先获取温度，用来补偿气压值  
  while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());    
  barometer.getTemperatureC();  
  barometer.setControl(BMP085_MODE_PRESSURE_3);   
  //得出气压值  
  while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());  
  return barometer.getPressure();
}

float getAlt()
{
  return barometer.getAltitude(getPre()); //结合气压值，以标准大气压得出海拔值  
}

char* c_Cache(float _f_Cache)
{
  char _c_Cache[20];
  dtostrf(_f_Cache,4,0,_c_Cache);  //将获取的数值转换为字符型数组
  return _c_Cache;
}

//================
//static char sendBuf[500];
boolean sendDATA(int _sendDATA)
{
  boolean _Weather=false;
  switch(_sendDATA)
  {
  case 1:
    SEND.print("Egg;");
    SEND.print(egg_UUID);
    SEND.print(';');
    SEND.print(TimeStamp,0);
    SEND.print(";");
    SEND.print(egg_mpu[0]);
    SEND.print(",");
    SEND.print(egg_mpu[1]);
    SEND.print(",");
    SEND.print(egg_mpu[2]);
    SEND.print(";");
    SEND.print(egg_mpu[3]);
    SEND.print(",");
    SEND.print(egg_mpu[4]);
    SEND.print(",");
    SEND.print(egg_mpu[5]);
    SEND.print(";;");
    SEND.println("");   
    break;
  case 2:
    //----------
    SEND.print("Egg;");
    SEND.print(egg_UUID);
    SEND.print(';');
    SEND.print(TimeStamp,0);
    SEND.print(";;;");
    for(int a=0;a<14;a++)
    {
      SEND.print(egg_tem[a]);
      if(a<14-1)
        SEND.print(",");
    }
    SEND.print(";");  
    SEND.println("");   

    //----------
    _Weather=true;
    break;

  case 3:
    //----------
    SEND.print("Egg;");
    SEND.print(egg_UUID);
    SEND.print(';');
    SEND.print(TimeStamp,0);
    SEND.print(";;;;");
    SEND.print(egg_hum[0]);  
    SEND.println("");   

    //----------
    _Weather=true;
    break;

  default:    
    return false;

  }

  if(_Weather)
  {
    SEND.print("Weather;");
    SEND.print(TimeStamp,0);
    SEND.print(";");
    SEND.print(sensor_tem);
    SEND.print(":");  
    SEND.print(sensor_hum);
    SEND.print(":");  
    SEND.print(sensor_lux,0);
    SEND.print(":");  
    SEND.print(sensor_pre,0);
    SEND.println("");   
  }

  return true;
}

void setup() 
{
  DEBUG.begin(115200);
  FROM.begin(115200);
  SEND.begin(115200);

  //  rtc.initClock();  //set a time to start with.
  //  rtc.setDate(21, 1, 7, 0, 14);  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  //rtc.setTime(2, 05, 30);  //hr, min, sec

  DEBUG.print("\n\rUnix TimeStamp:");
  DEBUG.println(getTimeStamp(1));

  //  Wire.begin();

  //初始化-气压   
  barometer.initialize();   
  DEBUG.println(barometer.testConnection() ? "BMP085 successful" : "BMP085 failed");   

  //初始化-光照   
  DEBUG.println(tsl.begin() ? "TSL2561 successful" : "TSL2561 failed");   
  tsl.enableAutoGain(true);                                  // 在1x 与16x 增益中切换  
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      //13MS的采样速度  
}


// the loop routine runs over and over again forever:
void loop() 
{

  if (FROM.available()>0) {
    inCache = inChar;
    inChar = FROM.read();

    pkj[num]=inChar;
    //DEBUG.print(inCache,HEX);
    //DEBUG.print(inChar,HEX);
    num++;
    delayMicroseconds(200);
  }

  if(sta)
  {
    sta=false;

    error=false;
    switch(inChar)
    {
    case 0xAA:
      type=1;
      DEBUG.println("\n\rType 1 RAW: ");
      break;
    case 0xBB:
      type=2;
      DEBUG.println("\n\rType 2 RAW: ");
      break;
    case 0xCC:
      type=3;
      DEBUG.println("\n\rType 3 RAW: ");
      break;
    default: 
      error=true;
      type=0;
      DEBUG.println("\n\rType ERROR ");
    }
    num=0;
  }

  if(inChar==0xbb && inCache==0xaa)
  {
    sta=true;
    DEBUG.println("\n\r");
    DEBUG.println("\n\r----START----");
  }

  if(inChar==0xff && inCache==0xff)
  {
    inChar=NULL;
    inCache=NULL;

    num-=2;

    for(long a=0;a<num;a++)
    {
      DEBUG.print(pkj[a],HEX);
      DEBUG.print(" ");
    }

    DEBUG.println(" ");

    if(error)
    {
      DEBUG.println("DATA ERROR");
    }
    else
    {
      DEBUG.println("DATA OK");
      read_data(num,pkj);

      //---------------------
      uint16_t time = millis();

      DEBUG.println("\n\rTIME---------------");

      TimeStamp=getTimeStamp(0);
      DEBUG.print("TimeStamp: ");
      DEBUG.print(TimeStamp);
      DEBUG.println(" \n\r");

      //---------------------
      if(type!=1)    //not mpu
      {
        sensor_tem=getTem();
        sensor_hum=getHum();
        sensor_lux=getLux();
        sensor_pre=getPre();
      }

      DEBUG.print("\n\rTEM: ");
      DEBUG.print(sensor_tem);
      DEBUG.print(",HUM: ");
      DEBUG.print(sensor_hum);
      DEBUG.print(",LUX: ");
      DEBUG.print(sensor_lux);
      DEBUG.print(",Pre: ");
      DEBUG.println(sensor_pre);

      time = millis() - time;
      DEBUG.print("[");
      DEBUG.print(time, DEC);
      DEBUG.print("]ms");
      DEBUG.println("------------");

      //---------------------
      DEBUG.println("\n\rsendDATA READY");
      if(sendDATA(type))
        DEBUG.println("sendDATA OK");
      else
        DEBUG.println("sendDATA ERROR");
    }
    //DEBUG.print("\n\r");
    //DEBUG.println(num);
    num=0;

    DEBUG.println("\n\r----END----");
  }
}
