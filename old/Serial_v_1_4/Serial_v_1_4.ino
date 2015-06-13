#define FROM Serial
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

int egg_mpu[6];
int egg_tem[14];
int egg_hum[1];

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
  int _bufin[_num];
  for(int i=0;i<_num;i++) {
    Serial.print("RC[");
    Serial.print(i+1);
    Serial.print("]:");

    Serial.print(_buf[2*i],DEC);
    Serial.print(",");
    Serial.print(_buf[2*i+1],DEC);

    Serial.print("   :");
    _bufin[i]=read16(_buf);
    Serial.println(_bufin[i]);
    //    delay(50);        // delay in between reads for stability
  }

  switch(type)
  {
  case 1: 
    Serial.print("DATA_MPU: ");
    for(int a=0;a<_num;a++)
    {
      egg_mpu[a]=_bufin[a];
      Serial.print(egg_mpu[a]);
      Serial.print(",");
    }
    break;
  case 2: 
    Serial.print("DATA_TEM: ");
    for(int a=0;a<_num;a++)
    {
      egg_tem[a]=_bufin[a];
      Serial.print(egg_tem[a]);
      Serial.print(",");
    }
    break;
  case 3: 
    Serial.print("DATA_HUM: ");
    for(int a=0;a<_num;a++)
    {
      egg_hum[a]=_bufin[a];
      Serial.print(egg_hum[a]);
      Serial.print(",");
    }
    break;
  }

  Serial.println(" ");
}

//send(HEX) :"AA BB CC 2A 3A 4A 5A 6A 7A 8A 9A FF FF"
unsigned long num=0;

byte by[512];

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
    DEBUG.print("\r\n");
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
//温湿度   
#include <OneWire.h>
//18B20
#define ONEWIRE_PIN 6
OneWire ds(ONEWIRE_PIN);

//气压   
#include "BMP085.h"                             //调用库  
BMP085 barometer;   
//光照   
#include <Adafruit_Sensor.h>                     //调用库  
#include <Adafruit_TSL2561_U.h>                 //调用库  
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);   

//3,传感器值的设置  
float sensor_tem, sensor_hum, sensor_alt, sensor_pre, sensor_lux; //温度、湿度、海拔、气压、光照   

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
  delay(100);
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


//================
void setup() 
{
  DEBUG.begin(115200);
  FROM.begin(115200);

  //  rtc.initClock();  //set a time to start with.
  //  rtc.setDate(21, 1, 7, 0, 14);  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  //rtc.setTime(2, 05, 30);  //hr, min, sec

  Serial.print("\r\nUnix TimeStamp:");
  Serial.println(getTimeStamp(1));


  //初始化-气压   
  barometer.initialize();   
  Serial.println(barometer.testConnection() ? "BMP085 successful" : "BMP085 failed");   

  //初始化-光照   
  Serial.println(tsl.begin() ? "TSL2561 successful" : "TSL2561 failed");   
  tsl.enableAutoGain(true);                                  // 在1x 与16x 增益中切换  
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      //13MS的采样速度  
}


// the loop routine runs over and over again forever:
void loop() 
{

  if (FROM.available()>0) {
    inCache = inChar;
    inChar = FROM.read();

    by[num]=inChar;
    //    DEBUG.write(inCache);
    //    DEBUG.write(inChar);
    if(inChar!=0xff)
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
      DEBUG.print("\n\rNO.1:[ ");
      break;
    case 0xBB:
      type=2;
      DEBUG.print("\n\rNO.2:[ ");
      break;
    case 0xCC:
      type=3;
      DEBUG.print("\n\rNO.3:[ ");
      break;
    default: 
      error=true;
      type=0;
      DEBUG.print("\n\rNO.ERROR ");
    }
    num=0;
  }

  if(inChar==0xbb && inCache==0xaa)
  {
    sta=true;
    DEBUG.println("\n\r----START----");
  }

  if(inChar==0xff && inCache==0xff)
  {
    inChar=NULL;
    inCache=NULL;

    for(long a=0;a<num;a++)
      DEBUG.write(by[a]);

    if(error)
    {
      DEBUG.println(" ]\n\rDATA ERROR");
    }
    else
    {
      DEBUG.println(" ]\n\rDATA OK");
      read_data(num,by);

      uint16_t time = millis();

      DEBUG.print("\n\rTIME: ");
      DEBUG.print("TimeStamp: ");
      DEBUG.print(getTimeStamp(0));
      DEBUG.println(" \n\r");
      sensor_tem=getTem();
      sensor_hum=getHum();
      sensor_lux=getLux();
      sensor_pre=getPre();

      DEBUG.print("\n\rTEM: ");
      DEBUG.println(sensor_tem);
      DEBUG.print("\n\rHUM: ");
      DEBUG.println(sensor_hum);
      DEBUG.print("\n\rLUX: ");
      DEBUG.println(sensor_lux);
      DEBUG.print("\n\rPre: ");
      DEBUG.println(sensor_pre);

      time = millis() - time;
      DEBUG.print("[");
      DEBUG.print(time, DEC);
      DEBUG.println("]ms");
      
    }
    //      DEBUG.print("\n\r");
    //      DEBUG.print(num);
    //      DEBUG.println("\n\rEND");
    num=0;

    DEBUG.println("\n\r----END----");
  }
}







