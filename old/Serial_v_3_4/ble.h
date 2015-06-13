#include "Arduino.h"

//==================
#define timeSwitch 100
unsigned long millisSwitch = millis();

boolean bleConnect(String _bleConnect)
{
  FROM.print("AT+CON");
  FROM.print(_bleConnect);
  delay(3000);
  
  return true;
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


