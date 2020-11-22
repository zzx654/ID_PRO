#include <SoftwareSerial.h>
SoftwareSerial bluetooth(4,5);
void setup() 
{
  Serial.begin(9600);
  bluetooth.begin(9600);
}
void loop()
{
 reception_process();
}
void reception_process()
{
  String medicdata="";
  while(Serial.available())
  {
    char myChar=(char)Serial.read();
    if(!isSpace(myChar))
     medicdata+=myChar;
    delay(15);
  }
  if(!medicdata.equals(""))
  {
    bluetooth.print(medicdata);
  }
}
