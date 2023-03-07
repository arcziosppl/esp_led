#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "EEPROM.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 200

AsyncWebServer server(80);

class led_strip
{
  private:
  uint8_t red,green,blue;

  public:
  void set_colors(uint8_t r, uint8_t g, uint8_t b){
    red = r;
    green = g;
    blue = b;
  }
};

led_strip LEDStrip;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LED</title>
  <script src="https://cdn.jsdelivr.net/npm/@jaames/iro@5"></script>
  <style>
body,html{
  background-color: black;
  width: 100ww;
  height: 100wh;
}

.background{
  height: 95%;
  width: 95%;
  background-color: #171F30;
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%,-50%);
  border-radius: 20px;
}

.background > .picker{
  position: fixed;
  left: 50%;
  transform: translateX(-50%);
}
  </style>
</head>
<body>
  <div class="background">
    <div class="picker"></div>
  </div>

  <script>
    const colorPicker = new iro.ColorPicker('.picker',{
      width: 300
    });

    colorPicker.on('color:change', (color)=>{
      let colors = [color.red,color.green,color.blue]
      console.log(colors);
      fetch("/color?"+ "value=" + colors[0] + "&value2=" + colors[1] + "&value3=" + colors[2],{
        method: "GET"
      })
    })
  </script>
</body>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(9600);
  pixels.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting...");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  EEPROM.begin(EEPROM_SIZE);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });


server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request){
 String color_red;
  String color_green;
  String color_blue;
    if(request->hasParam("value"))
    {
    color_red = request->getParam("value")->value();
    color_green = request->getParam("value2")->value();
    color_blue = request->getParam("value3")->value();

    LEDStrip.set_colors(color_red.toInt(), color_green.toInt(), color_blue.toInt());
    EEPROM.write(0, color_red.toInt());
    EEPROM.write(1, color_green.toInt());
    EEPROM.write(2, color_blue.toInt());
    EEPROM.commit();
    }
  
    request->send(200);
  });


  
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
 Serial.println(EEPROM.read(0));
}
