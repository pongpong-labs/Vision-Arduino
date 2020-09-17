#include <ArduinoJson.h>
#include <EURK_Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Time.h>
#include <stdlib.h>
using namespace std;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String str_mon[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
int Year, Month, Day, Hour, Minute, Second; 
const char* ssid = "hrb-211";
const char* password = "";
WiFiClient client;

void setup () {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return;
  }
  testfillcircle();
  Split(getTime(), ' ');
  StaticJsonDocument<200> doc;
  time_t baseTime =GetTimeT(Year,Month,Day,Hour,Minute,Second);
  doc["id"]="1";
  doc["status"]=true;
  doc["real_time"]="2020-09-14 10:59:59";
  doc["start_time"]="2020-09-14 10:59:59";
  doc["end_time"]="2020-09-14 10:59:59";
  String output;
  serializeJson(doc, output);
  Serial.println(output);
  if(WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String ubidots = "http://164.125.219.21:3000/api/arduino/1";
    http.begin(ubidots);
    int httpCode = http.PUT(output);
    Serial.println(httpCode);
    if (httpCode==200) {
      String payload = http.getString();
      char ch[99]={0};
      display.clearDisplay();
      payload.toCharArray(ch, payload.length());
      EURK_putsxy(0, 48, ch);
      display.display();
      Serial.println(payload);
    }
    http.end();
  }
  Serial.println(baseTime);
  Serial.println(ctime(&baseTime));
}

int GetTimeT(int YY, int MM, int DD, int hh, int mm, int ss) {  
    struct tm t = {0};
    t.tm_year = YY - 1900;
    t.tm_mon = MM - 1;
    t.tm_mday = DD;
    t.tm_hour = hh+9;
    t.tm_min = mm;
    t.tm_sec = ss;
    return mktime(&t);
}

//출처: https://kimgagakk.tistory.com/493 [김가가의 블로그]

void Split(String sData, char cSeparator)
{  
  int nCount = 0;
  int nGetIndex = 0;
  char str_Y[7]={0}, str_M[5]={0}, str_D[5]={0}, str_hms[11]={0}, thms[5]={0};
  String sTemp = "";
  String sCopy = sData;
  while(true)
  {
    nGetIndex = sCopy.indexOf(cSeparator);
    if(-1 != nGetIndex)
    {
      sTemp = sCopy.substring(0, nGetIndex);
      Serial.println( sTemp );
      if(nCount==1)
      {
        sTemp.toCharArray(str_D, sTemp.length()+1);
        Day=atoi(str_D);
      }
      else if(nCount==2)
      {
        for(int i=0 ; i<12 ; i++)
        {
          if(str_mon[i]==sTemp)
          {
            Month=i+1;
            break;
          }
        }
      }
      else if(nCount==3)
      {
        sTemp.toCharArray(str_Y, sTemp.length()+1);
        Year=atoi(str_Y);        
      }
      else if(nCount==4)
      {
        sTemp.toCharArray(str_hms, sTemp.length()+1);
        strncpy(thms, str_hms, 2);
        Hour=atoi(thms);
        strncpy(thms, str_hms+3, 2);
        Minute=atoi(thms);
        strncpy(thms, str_hms+6, 2);
        Second=atoi(thms);
      }
      sCopy = sCopy.substring(nGetIndex + 1);
    }
    else
    {
      Serial.println( sCopy );
      break;
    }
    ++nCount;
  }
}
void testfillcircle(void) {
  display.clearDisplay();
  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=3) {
    display.fillCircle(display.width() / 2, display.height() / 2, i, SSD1306_INVERSE);
    display.display();
    delay(80);
  }
  delay(1000);
}

String getTime() {
  while (!client.connect("naver.com", 80)) {}
  client.print("HEAD / HTTP/1.1\r\n\r\n");
  while(!client.available()) {}
  while(client.available()){
    if (client.read() == '\n') {
      if (client.read() == 'D') {
        if (client.read() == 'a') {
          if (client.read() == 't') {
            if (client.read() == 'e') {    
              if (client.read() == ':') {    
                client.read();
                String timeData = client.readStringUntil('\r');
                client.stop();
                return timeData;
              }
            }
          }
        }
      }
    }
  }
}

void loop(){
  delay(10000);
}
