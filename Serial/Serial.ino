//send(HEX) :"AA BB CC 2A 3A 4A 5A 6A 7A 8A 9A FF FF"
//receive(HEX):"AA BB CC 2A 3A 4A 5A 6A 7A 8A 9A"
unsigned long num=0;

byte by[512];

byte inChar,inCache;

boolean sta =false;

boolean error=false;

#define FROM Serial
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
    case 0xCC:
      DEBUG.print("\n\rNO.1:[ ");
      break;
    case 0xDD:
      DEBUG.print("\n\rNO.2:[ ");
      break;
    case 0xEE:
      DEBUG.print("\n\rNO.3:[ ");
      break;
    default: 
      error=true;
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
    }
    //      DEBUG.print("\n\r");
    //      DEBUG.print(num);
    //      DEBUG.println("\n\rEND");
    num=0;
  }  
}
