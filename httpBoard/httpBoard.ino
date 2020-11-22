#include<SoftwareSerial.h>
#include<Wire.h>
SoftwareSerial esp8266(7,8);
String SSID = "Project207";
String PASSWORD = "207207207";
String HOST = "13.209.88.95";
String PORT = "5000";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
String stamac = "                 ";
String FCM_URI="/push";
String RECORD_URI="/update";
String FAIL_URI = "/fail";
String data="";
boolean received;
void setup()
{
  // put your setup code here, to run once:
 Serial.begin(9600);
 Wire.begin(1);
 Wire.onReceive(receiveEvent);
 esp8266.begin(9600);
 get_macaddress();//회원정보,안드로이드기기 찾아가기위한 맥주소
 Serial.println(stamac);
 wifi_setup();

}

void loop() {
  // put your main code here, to run repeatedly:
  if(received)
  {
   server_request(data);
   data="";
   received=false;
  }
}
void receiveEvent(int parameter)
{
  while(Wire.available())
 {
   char ch=(char)Wire.read();
   if(!isSpace(ch))
   data+=ch;
   delay(15);
 }
 if(!data.equals(""))
 {
  received=true;
  Serial.println(data);
 }
}
void server_request(String data)
{
  char buf1[20];
  char buf2[20];
  int seperator=data.indexOf(",");
  int length=data.length();
  String sig=data.substring(0,seperator);
  String requestdata=data.substring(seperator+1,length);
  
  if(sig.equals("#"))//진통제 푸시알림
  {
    httpPost(FCM_URI,"CALLROBOT");
  }
  if(sig.equals("*"))//약복용 업데이트
  {
     httpPost(RECORD_URI,requestdata);
  }
  if(sig.equals("&"))//약복용실패 알림
  {
    httpPost(FAIL_URI,requestdata);
  }
}
void get_macaddress()
{
  int a = 0;
  char buf[100];
  esp8266.println("AT+CIFSR");//station 맥주소를 얻는명령어
  esp8266.readString().toCharArray(buf, 100); //문자열 버퍼에 응답데이터(맥주소를포함한 데이터)저장
  while (true)
  {
    if (buf[a] == 'S' && buf[a + 1] == 'T' && buf[a + 2] == 'A' && buf[a + 3] == 'M')
    {
      a += 8;
      for (int i = 0; i < 17; i++)
      {
        if(i==16)
        {
          stamac.setCharAt(i,'d');
          break;
        }
        stamac.setCharAt(i, buf[a]);
        a++;
      }
      break;
    }
    a++;
  }//문자열중 맥주소만 추출
}
void sendCommandToESP8266(String command, int maxTime, char readReplay[]) {//와이파이 접속실패를 방지하기 위한 함수

  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))//인자로 받은 maxtime만큼 반복하여 at명령어 수행
  {
    esp8266.println(command);
    if (esp8266.find(readReplay))//AT명령어에 따른 응답데이터중 원하는 데이터를 찾으면 찾은것으로 간주
    {
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if (found == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
}
void wifi_setup()
{
  sendCommandToESP8266("AT+RST", 5, "OK");
  sendCommandToESP8266("AT+CWMODE=1", 5, "OK");
  sendCommandToESP8266("AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"", 20, "OK");
  sendCommandToESP8266("AT+CIPMUX=1", 5, "OK");//와이파이 초기설정
}
void httpPost(String uri, String msg) //웹서버에 post요청을 전송하는 함수
{
  //esp8266.listen();
  Serial.println(uri);
  sendCommandToESP8266("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 3, "OK"); //AT명령어를 통해 웹서버에 연결
  String postdata = "";
  String key_1 = "msg=";
  String key_2 = "medicname=";
  postdata.concat(key_1);
  postdata.concat(stamac);//맥주소 데이터에 실음
  if (!msg.equals("CALLROBOT")) //fcm요청의 경우 mac주소만 전송하면 되므로 fcm이아닌 약복용정보 업데이트 요청시에만 body에 두개의 데이터를실음
  {
    postdata.concat("&");
    postdata.concat(key_2);
    postdata.concat(msg);//약이름
  }

  Serial.print("post data [");
  Serial.print(postdata);
  Serial.println("]");
  String postRequest =
    "POST " + uri + " HTTP/1.1\r\n" +
    "Host: " + HOST + "\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "Content-Length: " + postdata.length() + "\r\n" +
    "\r\n" +
    postdata + "\r\n";
  String cipSend = "AT+CIPSEND=0," + String(postRequest.length());
  sendCommandToESP8266(cipSend, 20, ">"); //post 요청 전처리
  esp8266.println(postRequest);//완성된 post 데이터 요청
  countTrueCommand++;
}
