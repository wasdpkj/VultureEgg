
#include "def.h"
#include "ble.h"

int egg_num = 1;

//===================
void setup()
{
#ifdef _DEBUG
  DEBUG.begin(115200);
#endif
  SEND.begin(115200);

  delay(1000);
  //======================
  if (bleBaud_speed(9600))
    DEBUG.println("bleBaud_speed ok");
  else
    DEBUG.println("bleBaud_speed error");

  /*
    if (bleRenew())
      DEBUG.println("bleReset ok");
    else
      DEBUG.println("bleReset error");
  */

  if (bleCon(egg_MAC))
    DEBUG.println("bleCon ok");
  else
    DEBUG.println("bleCon error");


  //bleInit();
  //bleClear();
  //bleBegin();

  while (SEND.available())
  {
    DEBUG.print(SEND.read());
  }
  delay(10);
}


// the loop routine runs over and over again forever:
void loop()
{
  if (Serial.available()) {
    char c = Serial.read();
    Serial1.write(c);
  }
  if (Serial1.available()) {
    char c = Serial1.read();
    Serial.write(c);
  }
}
