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
void setup() 
{
  DEBUG.begin(115200);
  FROM.begin(115200);

  //  rtc.initClock();  //set a time to start with.
  //  rtc.setDate(21, 1, 7, 0, 14);  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  //rtc.setTime(2, 05, 30);  //hr, min, sec

  Serial.print("\r\nUnix TimeStamp:");
  Serial.println(getTimeStamp(1));
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
      DEBUG.print("\n\rTIME: ");
      DEBUG.print("TimeStamp: ");
      DEBUG.print(getTimeStamp(0));
      DEBUG.println(" \n\r");
    }
    //      DEBUG.print("\n\r");
    //      DEBUG.print(num);
    //      DEBUG.println("\n\rEND");
    num=0;

    DEBUG.println("\n\r----END----");
  }  
}
