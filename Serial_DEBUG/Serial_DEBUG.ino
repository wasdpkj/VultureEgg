unsigned long time1=millis();

int type;

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

int egg_mpu[6];
int egg_tem[14];
int egg_hum[1];

int RCin[8],RCoutA[8],RCoutB[8];
int p;
uint16_t t,t1,t2;

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

    Serial.print(_buf[2*i],HEX);
    Serial.print(",");
    Serial.print(_buf[2*i+1],HEX);

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

#define FROM Serial1
#define DEBUG Serial

void setup() 
{
  DEBUG.begin(115200);
  FROM.begin(115200);
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

  if(inChar==0x0a && inCache==0x0d)
  {
    inChar=NULL;
    inCache=NULL;

    num-=2;

    for(long a=0;a<num;a++)
    {
      DEBUG.print(by[a],HEX);
      DEBUG.print(" ");
    }

    if(error)
    {
      DEBUG.println(" ]\n\rDATA ERROR");
    }
    else
    {
      DEBUG.println(" ]\n\rDATA OK");
      read_data(num,by);
    }
    //      DEBUG.print("\n\r");
    //      DEBUG.print(num);
    //      DEBUG.println("\n\rEND");
    num=0;

    DEBUG.println("\n\r----END----");
  }  
}




