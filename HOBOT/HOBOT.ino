#include <GP2Y0A02YK0F.h>
#include <Stepper.h>
#include <XBee.h>
#include<Wire.h>
#include<SoftwareSerial.h>
#define DIST_S 200*58.2 // 200cm로 제한
#define millisMAX 0XFFFFFFFF
#define NUMBER_OF_SPACE 5//약 수납공간개수변경시 이부분만 바꾸면 됨
#define TIME_UNIT 1000
#define TIME_LIMIT 20000//약 복용에 제한시간을 둔다
#define medic_threshold 8
#define complete_threshold 90
#define trackingmode 1
#define fillmode 0
#define GO 1
#define BACK 0
#define LEFT 1
#define RIGHT 2
#define CENTER 0
#define trackdistmax 115
#define trackdistmin 75
#define calleddistmax 30
#define calleddistmin 45
SoftwareSerial xb(10, 11); //dout,din
XBee xbeeL = XBee();//좌측 xbee객체 생성
XBee xbeeR = XBee();//우측 xbee객체 생성
GP2Y0A02YK0F irSensor;
int speakerpin = 7; //스피커가 연결된 디지털핀 설정
unsigned long now;
unsigned long prev;
boolean alarm;
#define PIEZO_INTERVAL 150
int note[] = {2093, 4186, 2793};
int i = 0;
int lidcount=0;
int elementCount = sizeof(note) / sizeof(int);
int rssiL, rssiR; //좌우 rssi값 저장 변수
int RightMotor_E_pin = 5;      // 오른쪽 모터의 Enable & PWM
int RightMotor_1_pin = 8;      // 오른쪽 모터 제어선 IN1
int RightMotor_2_pin = 9;     // 오른쪽 모터 제어선 IN2
int LeftMotor_3_pin = 4;      // 왼쪽 모터 제어선 IN3
int LeftMotor_4_pin = 3;      // 왼쪽 모터 제어선 IN4
int LeftMotor_E_pin = 6;      // 왼쪽 모터의 Enable & PWM
int CarSpeed = 55;
int band_direction;
long band_distance;
int car_direc = CENTER;
int direc_check;
int HOBOTMODE = fillmode;
int go_back = BACK;
int dist_min = trackdistmin;
int dist_max = trackdistmax;
String medic_name[NUMBER_OF_SPACE];
unsigned long medic_interval[NUMBER_OF_SPACE];
unsigned long medic_curtime[NUMBER_OF_SPACE];
unsigned long medic_pretime[NUMBER_OF_SPACE];
String medicdata;
String painkill = "painkiller";
boolean stopped;
boolean painkiller;
boolean take_medicine;
boolean medic_check;
int led_pins[NUMBER_OF_SPACE] = {23, 24, 25, 26, 27};
int ledpainkiller = 22;
boolean medication[NUMBER_OF_SPACE];
boolean fillmedic[NUMBER_OF_SPACE];
Rx16Response rxL = Rx16Response();
Rx16Response rxR = Rx16Response();
int trigPin=13;
int echoPin=12;
long medic_distance;
int rotate=80
void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);//xbee시리얼
  Serial2.begin(9600);//블루투스 시리얼
  Serial3.begin(9600);
  xb.begin(9600);
  xbeeL.setSerial(xb);//시리얼 할당
  xbeeR.setSerial(Serial1);//시리얼 할당
  irSensor.begin(A0);
  pinMode(RightMotor_E_pin, OUTPUT);        // 출력모드로 설정
  pinMode(RightMotor_1_pin, OUTPUT);
  pinMode(RightMotor_2_pin, OUTPUT);
  pinMode(LeftMotor_3_pin, OUTPUT);
  pinMode(LeftMotor_4_pin, OUTPUT);
  pinMode(LeftMotor_E_pin, OUTPUT);
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  Wire.begin();
  Hobot_Stop();

}

void loop()
{
  HOBOT();
  now = millis();
  if (diff_check(now, prev, PIEZO_INTERVAL))
  {
    prev = now;
    if (alarm)
    {
      tone(speakerpin, note[i], 200);
    }

    i = (i + 1) % elementCount;
  }

}
void Hobot_Right() // 우회전
{
  digitalWrite(RightMotor_1_pin, LOW); //오른쪽 역회전
  digitalWrite(RightMotor_2_pin, HIGH);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH); //왼쪽 정회전
  analogWrite(RightMotor_E_pin, CarSpeed);
  analogWrite(LeftMotor_E_pin, CarSpeed);
}
void Hobot_Stop() //정지
{
  digitalWrite(RightMotor_1_pin, LOW); //오른쪽 역회전
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, LOW); //왼쪽 정회전
  analogWrite(RightMotor_E_pin, 0);
  analogWrite(LeftMotor_E_pin, 0);
}
void Hobot_Go()  // 전진
{
  digitalWrite(RightMotor_1_pin, HIGH); //오른쪽 정회전
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH); //왼쪽 정회전
  analogWrite(RightMotor_E_pin, CarSpeed);
  analogWrite(LeftMotor_E_pin, CarSpeed);
}
void Hobot_Left()  // 좌회전
{
  digitalWrite(RightMotor_1_pin, HIGH); //오른쪽 정회전
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW); //왼쪽 역회전
  analogWrite(RightMotor_E_pin, CarSpeed);
  analogWrite(LeftMotor_E_pin, CarSpeed);
}

void Hobot_Back() //후진
{
  digitalWrite(RightMotor_1_pin, LOW); //오른쪽 정회전
  digitalWrite(RightMotor_2_pin, HIGH);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW); //왼쪽 정회전
  analogWrite(RightMotor_E_pin, CarSpeed);
  analogWrite(LeftMotor_E_pin, CarSpeed);
}
int get_direction()
{
  int direc;
  //좌측 rssi값 수신
  xbeeL.readPacket(100);
  if (xbeeL.getResponse().isAvailable())
  {
    if (xbeeL.getResponse().getApiId() == RX_16_RESPONSE)
    {
      xbeeL.getResponse().getRx16Response(rxL);
      rssiL = rxL.getRssi();
    }
  }
  //우측 rssi값 수신
  xbeeR.readPacket(100);
  if (xbeeR.getResponse().isAvailable())
  {
    if (xbeeR.getResponse().getApiId() == RX_16_RESPONSE)
    {
      xbeeR.getResponse().getRx16Response(rxR);
      rssiR = rxR.getRssi();
    }
  }
  //방향 파악
  if (abs(rssiL - rssiR) <= 8)
  {
    Serial.println("1@@@");
    direc = CENTER;
  }
  else if (rssiL < rssiR)
  {
    Serial.println("0@@@");
    direc = LEFT;
  }
  else if (rssiL > rssiR)
  {
    Serial.println("2@@@");
    direc = RIGHT;
  }

  return direc;
}
void HOBOT()
{
  periodic_check();
  reception_process();
  if (HOBOTMODE == trackingmode && !stopped)
  {
    Hobot_track();
  }
  else
  {
    band_distance=irSensor.getDistanceCentimeter();
    medic_distance=check_distance(trigPin,echoPin);
    Serial.println(medic_distance);

    if (stopped && medic_distance < medic_threshold&&medic_distance!=0) //환자 가까이 다가간 상태에서 약통 뚜껑이 열렸는지
    {
      if(lidcount++>10)
      {
        lidcount=0;
      take_medicine = true;
      medic_check = true;
       for (int i = 0; i < NUMBER_OF_SPACE; i++)
      {
        if (medication[i] == true) //어떤약 복용시간이었는지
        {
          alarm=false;//알람해제
        }
      }
      }
    }
    if (take_medicine && band_distance > complete_threshold) //약 복용후 간호로봇에게서 환자가 멀어졌는지를 검사
    {
      take_medicine = false;
      stopped = false;
      go_back = BACK;
      dist_min = trackdistmin;
      dist_max = trackdistmax;
      if (painkiller)
      {
        painkiller = false;
        Hobot_Stop();
        requestesp('*',painkill);//슬레이브보드로 서버요청신호전달
        digitalWrite(ledpainkiller, LOW);
      }
      for (int i = 0; i < NUMBER_OF_SPACE; i++)
      {
        if (medication[i] == true) //어떤약 복용시간이었는지
        {
          Serial.print(medic_name[i]);
          digitalWrite(led_pins[i], LOW);
          Hobot_Stop();
          requestesp('*',medic_name[i]);////슬레이브보드로 서버요청신호전달(*은 약봉용 업데이트 신호)
          medication[i] = false;
        }
      }

    }
  }
}
void Hobot_track()
{
  band_direction = get_direction();
 band_distance=irSensor.getDistanceCentimeter();
 Serial.println(band_distance);
  if (band_direction == LEFT)
  {
    Hobot_Left();
  }
  else if (band_direction == RIGHT)
  {
    Hobot_Right();
  }
  else if (band_direction == CENTER)
  {
    if (band_distance > dist_max)
    {
      Hobot_Go();
    }
    else if (band_distance >= dist_min && band_distance <= dist_max)
    {
      Hobot_Stop();
      if (go_back == GO && !stopped)//환자 호출또는 약 주기가 된 상태에서 환자와 적당한 거리에 있을때 멈춤
      {
        stopped = true;
      }
    }
    else
    {
      Hobot_Back();
    }

  }
}
void reception_process()
{
  String medicdata = "";
  while (Serial3.available())
  {
    char myChar = (char)Serial3.read();
    if (!isSpace(myChar)) //공백을 제외하고 문자열을 구성해야 문자열 비교가 가능
      medicdata += myChar;
    delay(20);
  }
  if (!medicdata.equals("")) //안드로이드->라즈베리파이,라즈베리파이->아두이노(시리얼통신)수신데이터(약주기 업데이트정보)가 있을시
  {
    Serial.println(medicdata);
    medic_update(medicdata);//수신데이터를 이용해 삭제하는지 등록또는 수정하는지 판단하여 업데이트수행
  }
  //호출밴드로부터 수신처리
  while (Serial2.available())
  {

    char ch = (char)Serial2.read();
    if (ch == 'c')//호출신호 수신시
    {
      if (HOBOTMODE == trackingmode)
      {
        if (go_back == BACK)
        {
          dist_min = calleddistmin;
          dist_max = calleddistmax;
          Hobot_Stop();
          digitalWrite(ledpainkiller, HIGH);
          painkiller = true;
          requestesp('#',painkill);//슬레이브보드로 요청(#은 진통제 푸시)
          go_back = GO;
          Serial.println("CALL Received!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
     
      }
    }
    else if (ch == 'm')//모드변경 버튼 수신시
    {
      if (HOBOTMODE == trackingmode)
      {
        HOBOTMODE = fillmode;
        Hobot_Stop();
      }
      else
      {
        HOBOTMODE = trackingmode;
      }
    }

  }
}
void medic_update(String medicdata)
{
  char buf1[20];
  char buf2[20];
  int seperator1 = medicdata.indexOf(",");// 첫 번째 콤마 위치
  int seperator2 = medicdata.indexOf(",", seperator1 + 1);
  int length = medicdata.length(); // 문자열 길이
  String token1, token2, token3;
  String medicname;
  unsigned long medicinterval;
  int medic_loc;
  token1 = medicdata.substring(0, seperator1); //첫번째 토큰 무조건 저장
  if (seperator2 >= 0) //등록신호가 들어왔을때
  {
    token2 = medicdata.substring(seperator1 + 1, seperator2);
    token3 = medicdata.substring(seperator2 + 1, length);
  }
  else//등록신호가 아닌 삭제,수정일때
  {
    token2 = medicdata.substring(seperator1 + 1, length);
  }

  if (token1.equals("*") && seperator2 < 0) //삭제신호
  {
    medicname = token2; //삭제신호이면 token2가 약이름
    del_medic(medicname);
  }
  else//삭제신호가 아니면 token1은 약이름 token2는 주기
  {
    medicname = token1;
    token2.toCharArray(buf1, 20);
    medicinterval = (unsigned long)(atoi(buf1)) * TIME_UNIT; //주기를 1000ms를 곱하여 초단위로 변경(시연용으로 초단위)
    if (seperator2 >= 0)
    {
      token3.toCharArray(buf2, 20);
      medic_loc = atoi(buf2);
      medic_add(medicname, medicinterval, medic_loc);
    }
    else
    {
      medic_change(medicname, medicinterval); //약수정,추가함수호출
    }
  }
}
void medic_add(String medicname, unsigned long medicinterval, int medic_loc)
{
  medic_loc-=2;
  medic_name[medic_loc] = medicname; //약이름 추가
  medic_interval[medic_loc] = medicinterval; //약 주기 추가
  medic_pretime[medic_loc] = millis(); //이 시점부터 약주기 순환시작
}
void medic_change(String medicname, unsigned long medicinterval) //수정,추가 함수
{
  //순회결과 약이있으면 약주기를 변경
  for (int i = 0; i < NUMBER_OF_SPACE; i++)
  {
    if (medic_name[i].equals(medicname))
    {
      medic_interval[i] = medicinterval;
      medic_pretime[i] = millis(); //약추가 시점부터 주기순환 시작
      break;
    }
  }

}
void periodic_check()
{
  for (int i = 0; i < NUMBER_OF_SPACE; i++)
  {
    if (medic_interval[i] != 0) //주기가 설정되어있는약을 찾음
    {
      medic_curtime[i] = millis();
      if (diff_check(medic_curtime[i], medic_pretime[i], medic_interval[i]))
      {
        lidcount=0;
        medic_pretime[i] = medic_curtime[i];
        go_back = GO; //호출/후진버튼 입력시 후진으로 동작할수있게끔 한다.
        //블루투스 print함수로 특정 문자를 호출밴드로 전송하여 부저 울리게
        medication[i] = true; //어떤약 복용해야할 시점인지 신호를준다.
        medic_check = false; //약복용체크 신호,약주기 순환 시작마다 초기화
        dist_min = calleddistmin;
        dist_max = calleddistmax;
        alarm = true;
        Serial.print(medic_name[i]);
        Serial.println(" 복용할 시간입니다!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!(LED켜고 부저켜기)");
        digitalWrite(led_pins[i], HIGH);

      }
      if (medication[i] == true && diff_check(medic_curtime[i], medic_pretime[i], TIME_LIMIT)) //약복용 시간이 된것을 제한시간내에 복용했는지 검사
      {
        if (medic_check == false) //제한시간안에 약을 복용하지 않으면(시연용20초)
        {
          lidcount=0;
          medication[i] = false; //약 복용 실패
          //서버푸쉬 요청
          requestesp('&',medic_name[i]);//호출밴드로 요청(&은 약복용실패 푸시요청)
          //블루투스 print함수로 특정 문자를 호출밴드로 전송하여 부저 끄게
          alarm = false;
          Serial.println("뚜껑닫고 부저 끄게하고 LED끄기 및 후진@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
          digitalWrite(led_pins[i], LOW);
          Serial.print(medic_name[i]);
          Serial.println("약 먹기 실패@@@@@@@@@@@");

          stopped = false;
          go_back = BACK;
          dist_min = trackdistmin;
          dist_max = trackdistmax;


        }

      }
    }
  }
}
void init_medic()//약 수납공간 개수만큼 약정보 초기화하는 함수
{
  for (int i = 0; i < NUMBER_OF_SPACE; i++)
  {
    medic_name[i] = "";
    medic_interval[i] = 0;
    medic_curtime[i] = 0;
    medic_pretime[i] = 0;
    fillmedic[i] = false;
    pinMode(led_pins[i], OUTPUT);
  }
}
void del_medic(String medicname)//약 삭제함수
{
  for (int i = 0; i < NUMBER_OF_SPACE; i++)
  {
    if (medic_name[i].equals(medicname)) //순회 결과 삭제하려는 약이름이 있을경우 삭제(이름,주기모두 초기화)
    {
      medic_name[i] = "";
      medic_interval[i] = 0;
      medic_curtime[i] = 0;
      medic_pretime[i] = 0;
      medication[i] = false;
      break;
    }
  }
}
boolean diff_check(unsigned long now, unsigned long prev, unsigned long interval) //롤오버시 주기체크에러 방지
{
  if (now >= prev)
  {
    return ((now - prev) >= interval);
  }
  else
  {
    return ((millisMAX - prev) + now >= interval); //음수로서 비교되지 않도록 연산통해 조건식의 결과값을 반환
  }
}
long check_distance(int t,int e)//초음파 함수
{
  long dist;
  digitalWrite(t, LOW); 
  delayMicroseconds(2); 
  digitalWrite(t, HIGH); 
  delayMicroseconds(10);
  digitalWrite(t, LOW);
  dist = pulseIn(e, HIGH,DIST_S)/58.2;
  return dist;
}
void requestesp(char c,String requestdata)
{
  Wire.beginTransmission(1);
  String data="";
  data+=c;//요청신호(푸시,복용업데이트등)를 뜻하는 문자합치기
  data.concat(",");//구분문자 합치기
  data.concat(requestdata);//약이름 합치기
  Serial.println(data);
    byte*temp=new byte[data.length()+1];
    data.getBytes(temp,data.length()+1);
    for(int i=0;i<data.length();i++)
    {
     Wire.write(*(temp+i));
    }
    Wire.endTransmission();
}
