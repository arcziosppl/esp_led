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
  public:
  uint8_t red,green,blue;
  bool stript_state = EEPROM.read(3);

  public:
  void set_colors(uint8_t r, uint8_t g, uint8_t b){
    red = r;
    green = g;
    blue = b;
  }

  void set_strip_color(){
     for(int i=0; i<NUMPIXELS; i++)
 {
pixels.setPixelColor(i, pixels.Color(red, green, blue));
 }
 pixels.show();
  }

  void set_strip_state(bool state){
    stript_state = state; //turn off or on LED strip (true == turn on, false == turn off) 
    EEPROM.write(3, state);
    EEPROM.commit();
    Serial.println(EEPROM.read(3));
    if(state){
      for(int i=0; i<NUMPIXELS; i++){
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
      }
      pixels.show();
    }else{
      pixels.clear();
      pixels.setPixelColor(NUMPIXELS, pixels.Color(0, 0, 0));
      pixels.show();
    }
    
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
body{
  background-color: #171F30;
  width: 100%;
  height: 100%;
}

.background{
  height: 95vh;
  width: 95vw;
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  background: rgba(19, 18, 18, 0.404);
box-shadow: 0 8px 32px 0 rgba( 31, 38, 135, 0.37 );
backdrop-filter: blur( 30px );
-webkit-backdrop-filter: blur( 20px );
border-radius: 10px;
display: flex;
justify-content: center;
overflow: hidden;
}

.background > .main{
  width: 300px;
}

.main > .picker{
  padding-top: 20px;
}
.main > .controlls{
  border-radius: 20px;
  margin-top: 20px;
  width: 300px;
  height: 50%;
  text-align: center;
}

.controlls > .state{
  border: none;
  background: none;
  cursor: pointer;
  margin-top: 10px;
}

.controlls > .state > img{
  width: 50px;
}
  </style>
</head>
<body>
  <div class="background">
    <div class="main">
    <div class="picker"></div>
    <div class="controlls">
      <button class="state"><img src="https://cdn-icons-png.flaticon.com/512/4023/4023190.png" alt="on off"></button>
    </div>
    </div>
  </div>

  <script>

    const state_btn = document.querySelector('.state');
    let state;

    const colorPicker = new iro.ColorPicker('.picker',{
      width: 300,
    });

    colorPicker.on('color:change', (color)=>{
      let colors = [color.red,color.green,color.blue];
      document.body.style.backgroundColor =  color.rgbString;
      fetch("/color?"+ "value=" + colors[0] + "&value2=" + colors[1] + "&value3=" + colors[2],{
        method: "GET"
      })
    })

    state_btn.addEventListener('click', ()=>{
      if(state === true){
        state = false;
        fetch('/on',{
          method: "GET"
        })
      }else{
        state = true;
        fetch('/off',{
          method: "GET"
        })
      }
    });

      const req  = async ()=>{
        let request = await fetch('/updateval');
        let res = await request.text();
        return res;
      }

      req().then((res)=>{
        let strip_state = res.split(',');
        console.log(strip_state)
        state = strip_state[0];
        colorPicker.color.rgb = { r: strip_state[1], g: strip_state[2], b: strip_state[3] };
      })



  </script>
</body>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
    EEPROM.begin(EEPROM_SIZE);
  Serial.begin(9600);
  pixels.begin();
    LEDStrip.set_colors(EEPROM.read(0), EEPROM.read(1), EEPROM.read(2));
    LEDStrip.set_strip_state(EEPROM.read(3));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting...");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

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
    LEDStrip.set_strip_color();
    EEPROM.write(0, color_red.toInt());
    EEPROM.write(1, color_green.toInt());
    EEPROM.write(2, color_blue.toInt());
    EEPROM.commit();
    }
  
    request->send(200);
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    LEDStrip.set_strip_state(true);
    request->send(200);
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    LEDStrip.set_strip_state(false);
    request->send(200);
  });

  server.on("/updateval", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(LEDStrip.stript_state) + "," + String(LEDStrip.red) + "," + String(LEDStrip.green) + "," + String(LEDStrip.blue));
  });


  
  server.onNotFound(notFound);
  server.begin();
}

void loop() {

}
