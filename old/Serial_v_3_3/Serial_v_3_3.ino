#define _DEBUG
#define FROM Serial1
#define SEND Serial
#define DEBUG Serial

#include "rtc.h"
//======================

/*
AA BB AA 1A 01 2A 01 3A 01 4A 01 5A 01 6A 01 FF FF
 */

/*
AA BB BB 1A 01 2A 01 3A 01 4A 01 5A 01 6A 01 7A 01 8A 01 9A 01 1B 02 2B 02 3B 02 4B 02 5B 02 FF FF
 */

/*
AA BB CC 1A 01 FF FF
 */

//-------------------------
#define egg_num_MAX 1

String egg_MAC[egg_num_MAX] = {
  "BC6A29BD0E6A"
};
unsigned int egg_num = 0;

unsigned long millis1 = millis();
int type;

float egg_mpu[7];
float egg_tem[16];
float egg_hum[1];

//=============================
int p;

int16_t read16(byte* _Buf) {
  byte _Buf1 = _Buf[p++];
  byte _Buf2 = _Buf[p++];
  int16_t r = (_Buf1 & 0xff) | ((_Buf2 << 8) & 0xff00);
  return r;
}

int8_t read8(byte* _Buf) {
  return _Buf[p++];
}

void read_data(int _num, byte* _buf)
{
  p = 0;
  _num = _num / 2;

  int16_t _bufin[_num];

  //  inBuf=_buf;
  for (int i = 0; i < _num; i++)
  {
    //    if(type==3)
    //      _bufin_u[i]=read16(_buf);
    //    else
    _bufin[i] = read16(_buf);
  }


#ifdef _DEBUG
  DEBUG.println("\n\r");
#endif

  switch (type)
  {
    case 1:
      {
#ifdef _DEBUG
        DEBUG.print("DATA_MPU: ");
#endif
        for (int a = 0; a < _num; a++)
        {
#ifdef _DEBUG
          DEBUG.print(_bufin[a]);
          DEBUG.print(",");
#endif
          egg_mpu[a] = _bufin[a];
        }
      }
      break;
    case 2:
      {
#ifdef _DEBUG
        DEBUG.print("DATA_TEM: ");
#endif
        for (int a = 0; a < _num; a++)
        {
          //        _bufin_u[a]=_bufin[a];
          //        egg_tem[a]=-6.0f + 125.0f * (float)((float)_bufin_u[a]/(float)65535);
          //        egg_tem[a]=_bufin[a];
          //          egg_tem[a] = int((((_bufin[(2*a)]) << 8) | ((_bufin[(2*a)+1])))) / 256.0F;
          egg_tem[a] = ((_bufin[a] >> 5) * 0.125);
#ifdef _DEBUG
          DEBUG.print(egg_tem[a]);
          if (a < _num - 1)
            DEBUG.print(",");
#endif
        }
      }
      break;
    case 3:
      {
        uint16_t _bufin_u[_num];
#ifdef _DEBUG
        DEBUG.print("DATA_HUM: ");
#endif
        for (int a = 0; a < _num; a++)
        {
          _bufin_u[a] = _bufin[a];
          egg_hum[a] = -6.0f + 125.0f * (float)((float)_bufin_u[a] / (float)65535);
#ifdef _DEBUG
          DEBUG.print(egg_hum[a]);
          if (a < _num - 1)
            DEBUG.print(",");
#endif
        }
      }
      break;
    default:
      {
#ifdef _DEBUG
        DEBUG.print("DATA_ERROR: ");
#endif
      }
  }

#ifdef _DEBUG
  DEBUG.println(" ");
#endif
}

unsigned long num = 0;

byte pkj[512];

byte inChar, inCache;

boolean sta = false;
boolean error = false;

//================
#include <Wire.h>
#include <Rtc_Pcf8563.h>

uint32_t TimeStamp;

//================
#include <Wire.h>                                 //调用库  
#include "I2Cdev.h"                             //调用库  
//温度
#include <lm75.h>
TempI2C_LM75 termo = TempI2C_LM75(0x48, TempI2C_LM75::nine_bits);
//湿度
#include <SHT2x.h>
//气压
#include "BMP085.h"                             //调用库  
BMP085 barometer;
//光照
#include <Adafruit_TSL2561_U.h>                 //调用库  
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);

//3,传感器值的设置
float sensor_tem, sensor_hum, sensor_lux, sensor_pre, sensor_alt; //温度 湿度 光照 气压 海拔

float getTem()
{
  return termo.getTemp();
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
  if (event.light)
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
  dtostrf(_f_Cache, 4, 0, _c_Cache); //将获取的数值转换为字符型数组
  return _c_Cache;
}

//================
//static char sendBuf[500];
boolean sendDATA(int _sendDATA, String _egg_MAC)
{
  boolean _Weather = false;
  switch (_sendDATA)
  {
    case 1:
      SEND.print("Egg;");
      SEND.print(_egg_MAC);
      SEND.print(';');
      SEND.print(TimeStamp);
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
      SEND.print(",");
      SEND.print(egg_mpu[6]);
      SEND.print(";;");
      SEND.println("");
      break;
    case 2:
      //----------
      SEND.print("Egg;");
      SEND.print(_egg_MAC);
      SEND.print(';');
      SEND.print(TimeStamp);
      SEND.print(";;;");
      for (int a = 0; a < 16; a++)
      {
        SEND.print(egg_tem[a]);
        if (a < 16 - 1)
          SEND.print(",");
      }
      SEND.print(";");
      SEND.println("");

      //----------
      _Weather = true;
      break;

    case 3:
      //----------
      SEND.print("Egg;");
      SEND.print(_egg_MAC);
      SEND.print(';');
      SEND.print(TimeStamp);
      SEND.print(";;;;");
      SEND.print(egg_hum[0]);
      SEND.println("");

      //----------
      _Weather = true;
      break;

    default:
      return false;

  }

  if (_Weather)
  {
    SEND.print("Weather;");
    SEND.print(TimeStamp);
    SEND.print(";");
    SEND.print(sensor_tem);
    SEND.print(";");
    SEND.print(sensor_hum);
    SEND.print(";");
    SEND.print(sensor_lux, 0);
    SEND.print(";");
    SEND.print(sensor_pre, 0);
    SEND.println("");
  }

  return true;
}

//==================
#define timeSwitch 100
unsigned long millisSwitch = millis();

boolean bleConnect(String _bleConnect)
{
  FROM.print("AT+CON");
  FROM.print(_bleConnect);
  delay(3000);
}

void bleBegin()
{
  FROM.print("AT+ROLE1");
  delay(500);
  FROM.print("AT+IMME1");
  delay(500);
  FROM.print("AT+RESET");
  delay(2000);
}

void bleBaud(boolean _Baud)
{
  if (!_Baud)
  {
    FROM.begin(9600);
    delay(500);
    FROM.print("AT+BAUD4");
    delay(500);
    FROM.print("AT+RESET");
    delay(2000);
  }
  else
  {
    FROM.begin(115200);
    delay(500);
  }
}

void bleInit()
{
  FROM.print("AT+IMME0");
  delay(500);
}

void bleClear()
{
  FROM.print("AT+CLEAR");
  delay(500);
}

//===================
void setup()
{
#ifdef _DEBUG
  DEBUG.begin(115200);
#endif
  FROM.begin(115200);
  SEND.begin(115200);


  //getDateStamp(getTimeStamp(13, 52, 33, 12, 6, 2015)); //Hour,Minute,Second,Day,Month,Year
  //Serial.println(getTimeStamp(Hour, Minute, Second, Day, Month, Year));
  //setRTC();

  //rtc.initClock();  //set a time to start with.
  //rtc.setDate(30, 2, 9, 0, 14);  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  //rtc.setTime(16, 12, 30);  //hr, min, sec

#ifdef _DEBUG
  DEBUG.print("\n\rUnix TimeStamp:");
  getRTC();
  DEBUG.println(getTimeStamp(Hour, Minute, Second, Day, Month, Year));
#endif


  //  Wire.begin();

  //初始化-温度
  DEBUG.println("LM75 Set resolution to ten bits ");
  termo.setResolution(TempI2C_LM75::ten_bits);

  //初始化-气压
  barometer.initialize();
  if (barometer.testConnection())
  {
#ifdef _DEBUG
    DEBUG.println("BMP085 successful");
#endif
  }
  else
  {
#ifdef _DEBUG
    DEBUG.println("BMP085 failed");
#endif
  }

  //初始化-光照
  if (tsl.begin())
  {
#ifdef _DEBUG
    DEBUG.println("TSL2561 successful");
#endif
  }
  else
  {
#ifdef _DEBUG
    DEBUG.println("TSL2561 failed");
#endif
  }
  tsl.enableAutoGain(true);                                  // 在1x 与16x 增益中切换
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      //13MS的采样速度

  //======================
  //bleBaud(1);
  //bleInit();
  //bleClear();
  //bleBegin();

  if (!bleConnect(egg_MAC[egg_num]))   //bleBegin
  {
#ifdef _DEBUG
    DEBUG.println("\n\r");
    DEBUG.print("[");
    DEBUG.print(egg_MAC[egg_num]);
    DEBUG.println("] ");
    DEBUG.println("connect ERROR");
#endif
    egg_num++;
  }
  else
  {
#ifdef _DEBUG
    DEBUG.println("\n\r");
    DEBUG.println("connect OK");
#endif
  }
  while (SEND.available())
  {
    DEBUG.print(SEND.read());
  }
  delay(10);
}


// the loop routine runs over and over again forever:
void loop()
{
  {
    if (FROM.available() > 0) {
      inCache = inChar;
      inChar = FROM.read();

      pkj[num] = inChar;
#ifdef _DEBUG
      //DEBUG.print(inCache,HEX);
      //DEBUG.print(inChar,HEX);
#endif
      num++;
      delayMicroseconds(200);
    }

    if (sta)
    {
      sta = false;

      error = false;
      switch (inChar)
      {
        case 0xAA:
          type = 1;
#ifdef _DEBUG
          DEBUG.println("\n\rType 1 RAW: ");
#endif
          break;
        case 0xBB:
          type = 2;
#ifdef _DEBUG
          DEBUG.println("\n\rType 2 RAW: ");
#endif
          break;
        case 0xCC:
          type = 3;
#ifdef _DEBUG
          DEBUG.println("\n\rType 3 RAW: ");
#endif
          break;
        default:
          error = true;
          type = 0;
#ifdef _DEBUG
          DEBUG.println("\n\rType ERROR ");
#endif
      }
      num = 0;
    }

    if (inChar == 0xbb && inCache == 0xaa)
    {
      sta = true;
#ifdef _DEBUG
      DEBUG.println("\n\r");
      DEBUG.println("\n\r----START----");
#endif
    }

    if (inChar == 0x0a && inCache == 0x0d)
    {
      inChar = NULL;
      inCache = NULL;

      num -= 2;

#ifdef _DEBUG
      DEBUG.print("NUM[");
      DEBUG.print(num);
      DEBUG.print("]:");
      for (long a = 0; a < num; a++)
      {
        DEBUG.print(pkj[a], HEX);
        DEBUG.print(" ");
      }
      DEBUG.println(" ");
#endif

      if (error)
      {
#ifdef _DEBUG
        DEBUG.println("DATA ERROR");
#endif
      }
      else
      {
#ifdef _DEBUG
        DEBUG.println("DATA OK");
#endif
        read_data(num, pkj);

        //---------------------
        int16_t millis2 = millis();

#ifdef _DEBUG
        DEBUG.println("\n\rTIME---------------");
#endif

        getRTC();
        TimeStamp = getTimeStamp(Hour, Minute, Second, Day, Month, Year);
#ifdef _DEBUG
        DEBUG.print("TimeStamp: ");
        DEBUG.print(TimeStamp);
        DEBUG.println(" \n\r");
#endif

        //---------------------
        if (type != 1) //not mpu
        {
          sensor_tem = getTem();
          sensor_hum = getHum();
          sensor_lux = getLux();
          sensor_pre = getPre();

#ifdef _DEBUG
          DEBUG.print("\n\rTEM: ");
          DEBUG.print(sensor_tem);
          DEBUG.print(",HUM: ");
          DEBUG.print(sensor_hum);
          DEBUG.print(",LUX: ");
          DEBUG.print(sensor_lux);
          DEBUG.print(",Pre: ");
          DEBUG.println(sensor_pre);
#endif
        }


#ifdef _DEBUG
        millis2 = millis() - millis2;
        DEBUG.print("[");
        DEBUG.print(millis2, DEC);
        DEBUG.print("]ms");
        DEBUG.println("------------");


        //---------------------
        DEBUG.println("\n\rsendDATA READY");
#endif

        if (sendDATA(type, egg_MAC[egg_num]))
        {
#ifdef _DEBUG
          DEBUG.println("sendDATA OK");
#endif
        }
        else
        {
#ifdef _DEBUG
          DEBUG.println("sendDATA ERROR");
#endif
        }
      }
#ifdef _DEBUG
      //DEBUG.print("\n\r");
      //DEBUG.println(num);
#endif
      num = 0;

#ifdef _DEBUG
      DEBUG.println("\n\r----END----");
#endif
    }
  }
  //===========================
  if (egg_num_MAX > 1) //egg_num_MAX>1
  {
    if (millis() < millisSwitch) millisSwitch = millis();
    if (millis() - millisSwitch > timeSwitch)
    {
      if (egg_num < egg_num_MAX)
        egg_num++;
      else
        egg_num = 0;

      if (!bleConnect(egg_MAC[egg_num]))
      {
#ifdef _DEBUG
        DEBUG.print("\n\r[");
        DEBUG.print(egg_MAC[egg_num]);
        DEBUG.println("\n\r] connect ERROR");
#endif
        egg_num++;
      }

      delay(10);

      millisSwitch = millis();
    }
  }
}
