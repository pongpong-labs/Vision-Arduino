#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Time.h>
#include <stdlib.h>
using namespace std;

String str_mon[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}, YStamp="", MStamp="", DStamp="", hmsStamp="";
int Year, Month, Day, Hour, Minute, Second; 
const char* ssid = "pongponglabs";
const char* password = "simple1231";

String startTime, endTime;

SoftwareSerial hm10(D5,D6); //RX, TX 연결
const int BUTTON_PIN = D2;
const int LED_PIN = D1;
int lastState = HIGH;
int currentState;
bool state=0;

WiFiClient client;

void setup () {
  Serial.begin(9600);
  hm10.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
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
      Serial.println(sTemp);
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
 
void loop() {
  currentState = digitalRead(BUTTON_PIN);
  delay(50);
  if(lastState == LOW && currentState == HIGH)
  {
    if(state)
    {
      state=false;
      digitalWrite(LED_PIN, LOW);
    }
    else
    {
      state=true;
      digitalWrite(LED_PIN, HIGH);
      Split(getTime(), ' ');
      time_t baseTime =GetTimeT(Year,Month,Day,Hour,Minute,Second);
      String TimeS=ctime(&baseTime);
      char temp[30]={0};
      Serial.println(TimeS);
      TimeS.toCharArray(temp, TimeS.length()+1);
      mktimeStamp(ctime(&baseTime), ' ');
      Serial.println(DStamp.length());
      if(DStamp.length()==2)
      {
        YStamp=String(temp[20])+String(temp[21])+String(temp[22])+String(temp[23]);
      }
      else
      {
        YStamp=String(temp[19])+String(temp[20])+String(temp[21])+String(temp[22]);
      }
      String timeStamp=YStamp+"-"+MStamp+"-"+DStamp+" "+hmsStamp;
      startTime=timeStamp;
      Serial.println(timeStamp);
      StaticJsonDocument<200> doc;
      doc["id"]="1";
      doc["name"]="김해CGV7관";
      doc["connect_time"]=startTime;
      doc["disconnect_time"]=startTime;
      doc["start_time"]=startTime;
      doc["end_time"]=startTime;
      doc["status"]=true;
      String output;
      serializeJson(doc, output);
      Serial.println(output);
      if(WiFi.status() == WL_CONNECTED)
      {
        HTTPClient http;
        String ubidots = "http://164.125.219.21:3000/api/beacon/1";
        http.begin(ubidots);
        http.addHeader("Content-Type", "application/json; charset=utf-8");
        int httpCode = http.PUT(output);
        Serial.println(httpCode);
        if (httpCode>0) {
          String payload = http.getString();
          Serial.println(payload);
        }
        http.end();
      }
    }
  }
  lastState = currentState;
}
