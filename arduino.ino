#include <EURK_Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
const char* ssid = "hrb-426";
const char* password = "";
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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

void loop(){
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String ubidots = "http://jsonplaceholder.typicode.com/todos/1";
    http.begin(ubidots);
    int httpCode = http.GET();
    Serial.print(httpCode);
    if (httpCode==200) {
      String payload = http.getString();
      char ch[99]={0};
      display.clearDisplay();
      payload.toCharArray(ch, payload.length());
      EURK_putsxy(0, 0, ch);
      display.display();
      Serial.println(payload);
    }
    http.end();
  }
  delay(10000);
}
