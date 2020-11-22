#include<XBee.h>
#include<SoftwareSerial.h>
#define millisMAX 0XFFFFFFFF
#define PING_INTERVAL 50
XBee xbee=XBee();
SoftwareSerial xb1(10,11);
SoftwareSerial BT(7,8);
uint8_t payload[] = { 'H', 'i' };
unsigned long pingPre;
unsigned long pingCur;
const int duration = 1000;
unsigned long pre_time1 = 0;
unsigned long cur_time1 = 0;
unsigned long pre_time2 = 0;
unsigned long cur_time2 = 0;
boolean btn1,btn2;
Tx16Request tx16 = Tx16Request(0xFFFF, payload, sizeof(payload));

void setup()
{
  Serial.begin(9600);
  xb1.begin(9600);
  BT.begin(9600);
  xbee.setSerial(xb1);
  attachInterrupt(0,pushBtn1,FALLING);
  attachInterrupt(1,pushBtn2,FALLING);
}

void loop()
{
    pingCur=millis();
    if(diff_check(pingCur,pingPre,PING_INTERVAL))
    {
     xbee.send( tx16 );
     pingPre=pingCur;
    }
    if(btn1)
    {
     btn1=false;
     BT.print("c");
    }
    if(btn2)
    {
     btn2=false;
     BT.print("m");
    }
    
}

boolean diff_check(unsigned long now,unsigned long prev,unsigned long interval)//롤오버시 주기체크에러 방지
{
  if(now>=prev)
  {
    return ((now-prev)>=interval);
  }
  else
  {
    return ((millisMAX-prev)+now>=interval);
  }
}
void pushBtn1(){
  cur_time1 = millis();  
  if(diff_check(cur_time1,pre_time1,duration)){    
    Serial.println("호출/후진버튼 인터럽트 발생!!");
    pre_time1 = cur_time1;
    btn1=true; 
  }
}
void pushBtn2(){
  cur_time2 = millis();  
  if(diff_check(cur_time2,pre_time2,duration)){    
    Serial.println("모드변경버튼 인터럽트 발생!!");
    pre_time2 = cur_time2;
    btn2=true;
  }
}
