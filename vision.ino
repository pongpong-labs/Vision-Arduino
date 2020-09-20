#include <ArduinoJson.h>
#include <EURK_Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_SSD1306.h>
#include <Time.h>
#include <stdlib.h>
#include <SoftwareSerial.h>
using namespace std;

SoftwareSerial HM10(D5,D6); //RX,TX
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String str_mon[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}, YStamp="", MStamp="", DStamp="", hmsStamp="";
int Year, Month, Day, Hour, Minute, Second; 
const char* ssid = "pongponglabs";
const char* password = "simple1231";
WiFiClient client;

void setup () {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected!");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return;
  }
  testfillcircle();
  delay(6000);
  Split(getTime(), ' ');
  time_t baseTime =GetTimeT(Year,Month,Day,Hour,Minute,Second);
  String TimeS=ctime(&baseTime);
  char temp[30]={0};
  TimeS.toCharArray(temp, TimeS.length()+1);
  mktimeStamp(ctime(&baseTime), ' ');
  if(DStamp.length()==2)
  {
    YStamp=String(temp[20])+String(temp[21])+String(temp[22])+String(temp[23]);
  }
  else
  {
    YStamp=String(temp[19])+String(temp[20])+String(temp[21])+String(temp[22]);
  }
  String timeStamp=YStamp+"-"+MStamp+"-"+DStamp+" "+hmsStamp;
  Serial.println(timeStamp);
  StaticJsonDocument<200> doc;
  doc["id"]="1";
  doc["real_time"]=timeStamp;
  String output;
  serializeJson(doc, output);
  Serial.println(output);
  if(WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("http://164.125.219.21:3000/api/arduino");
    http.addHeader("Content-Type", "application/json; charset=utf-8");
    int httpCode = http.POST(output);
    Serial.println(httpCode);
    if (httpCode>0) {
      String payload = http.getString();
      Serial.println(payload);
    }
    http.end();
    delay(50);
  }
  while(WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("http://164.125.219.21:3000/api/beacon");
    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode>0) {
      String payload = http.getString();
      StaticJsonDocument<600> JSONBuffer;
      DeserializationError error = deserializeJson(JSONBuffer, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
      String tmp=JSONBuffer["result"];
      Serial.println(tmp);
      StaticJsonDocument<400> buff;
      DeserializationError error2 = deserializeJson(buff, tmp);
      if (error2) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error2.c_str());
        return;
      }
      bool stat=buff["status"];
      String test=buff["name"];
      char ch[120]={0};
          display.clearDisplay();
          //display.ssd1306_command (0xA1);
          payload.toCharArray(ch, test.length());
          EURK_putsxy(0, 48, ch);
          display.display();
      Serial.println(stat);
      if(stat)
      {
        http.end();
        http.begin("http://164.125.219.21:3000/api/subtitles");
        httpCode = http.GET();
        Serial.println(httpCode);
        if(httpCode>0) {
          String subt = http.getString();
          StaticJsonDocument<800> subs;
          DeserializationError error3 = deserializeJson(subs, subt);
          if (error3) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error3.c_str());
            return;
          }
          String sub=subs["result"];
          char ch[120]={0};
          display.clearDisplay();
          //display.ssd1306_command (0xA1);
          payload.toCharArray(ch, sub.length());
          EURK_putsxy(0, 48, ch);
          display.display();
          Serial.println(payload);
        }
        http.end();
      }
    }
    else{
      http.end();
    }
  }
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
      break;
    }
    ++nCount;
  }
}

void mktimeStamp(String sData, char cSeparator)
{  
  int nCount = 0;
  int nGetIndex = 0;
  String sTemp = "";
  String sCopy = sData;
  while(true)
  {
    nGetIndex = sCopy.indexOf(cSeparator);
    if(-1 != nGetIndex)
    {
      sTemp = sCopy.substring(0, nGetIndex);
      if(nCount==2)
      {
        DStamp=sTemp;
      }
      else if(nCount==1)
      {
        for(int i=0 ; i<12 ; i++)
        {
          if(str_mon[i]==sTemp)
          {
            if(i<10)
            {
              MStamp="0"+String(i+1);
            }
            else
            {
              MStamp=String(i+1);
            }
            break;
          }
        }
      }
      else if(nCount==3)
      {
        hmsStamp=sTemp;
      }
      sCopy = sCopy.substring(nGetIndex + 1);
    }
    else
    {
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
  
  delay(50);
}
