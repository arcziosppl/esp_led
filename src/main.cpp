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
  bool rainbow_state = false;

  public:
  void set_colors(uint8_t r, uint8_t g, uint8_t b){
    red = r;
    green = g;
    blue = b;
  }

  void set_strip_color(){
    pixels.clear();
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

  void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<NUMPIXELS; i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / NUMPIXELS);
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show();
    delay(wait);
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
  <link rel="shortcut icon" href="https://raw.githubusercontent.com/arcziosppl/esp_led/main/src/led-diode-16.ico" type="image/x-icon">
  <style>
body{
  background-color: #171F30;
  width: 100%;
  height: 100%;
}

.background{
  height: 95%;
  width: 95%;
  position: fixed;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  background: rgba(19, 18, 18, 0.404);
backdrop-filter: blur( 30px );
-webkit-backdrop-filter: blur( 20px );
border-radius: 20px;
display: flex;
justify-content: center;
overflow: hidden;
}

.background > .main{
  width: 300px;
}

.main > .picker{
  display: flex;
  padding-top: 20px;
  justify-content: center;
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

.button {
 display: inline-block;
 width: 80px;
 height: 30px;
 background-color: #fff;
 border-radius: 30px;
 cursor: pointer;
 padding: 0;
}

#toggle {
 display: none;
}

.slider {
 display: block;
 font-size: 10px;
 position: relative;
}

.slider::after {
 content: 'OFF';
 width: 40px;
 height: 40px;
 background-color: #e03c3c;
 border: 2px solid #fff;
 border-radius: 50%;
 box-shadow: 0 0 5px rgba(0, 0, 0, .25);
 position: absolute;
 top: -5px;
 left: 0;
 display: grid;
 place-content: center;
 line-height: 0;
 transition: background-color .25s, transform .25s ease-in;
}

#toggle:checked + .slider::after {
 content: 'ON';
 background-color: #05ae3e;
 transform: translateX(40px) rotate(360deg);
}

button {
  display: block;
  position: relative;
  top: 30px;
  left: 50%;
  transform: translateX(-50%);
  padding: 10px 20px;
  border-radius: 7px;
  border: 1px solid rgb(61, 106, 255);
  font-size: 14px;
  text-transform: uppercase;
  font-weight: 600;
  letter-spacing: 2px;
  background: transparent;
  color: #fff;
  overflow: hidden;
  box-shadow: 0 0 0 0 transparent;
  -webkit-transition: all 0.2s ease-in;
  -moz-transition: all 0.2s ease-in;
  transition: all 0.2s ease-in;
}

button:hover {
  background: rgb(61, 106, 255);
  box-shadow: 0 0 30px 5px rgba(0, 142, 236, 0.815);
  -webkit-transition: all 0.2s ease-out;
  -moz-transition: all 0.2s ease-out;
  transition: all 0.2s ease-out;
}

button:hover::before {
  -webkit-animation: sh02 0.5s 0s linear;
  -moz-animation: sh02 0.5s 0s linear;
  animation: sh02 0.5s 0s linear;
}

button::before {
  content: '';
  display: block;
  width: 0px;
  height: 86%;
  position: absolute;
  top: 7%;
  left: 0%;
  opacity: 0;
  background: #fff;
  box-shadow: 0 0 50px 30px #fff;
  -webkit-transform: skewX(-20deg);
  -moz-transform: skewX(-20deg);
  -ms-transform: skewX(-20deg);
  -o-transform: skewX(-20deg);
  transform: skewX(-20deg);
}

@keyframes sh02 {
  from {
    opacity: 0;
    left: 0%;
  }

  50% {
    opacity: 1;
  }

  to {
    opacity: 0;
    left: 100%;
  }
}

button:active {
  box-shadow: 0 0 0 0 transparent;
  -webkit-transition: box-shadow 0.2s ease-in;
  -moz-transition: box-shadow 0.2s ease-in;
  transition: box-shadow 0.2s ease-in;
}

  </style>
</head>
<body>
  <div class="background">
    <div class="main">
    <div class="picker"></div>
    <div class="controlls">
      <label class="button" for="toggle">
        <input class="state" id="toggle" type="checkbox">
        <span class="slider"></span>
      </label>
      <button class="rainbow">Rainbow</button>
    </div>
    <div class="menu"></div>
    </div>
  </div>

  <script>

    const state_btn = document.querySelector('.state');
    const rainbow_btn = document.querySelector('.rainbow');
    let state = true;

    const colorPicker = new iro.ColorPicker('.picker',{
      width: 300,
    });

    if(window.innerWidth < 300){
      colorPicker.resize(250)
    }else{
      colorPicker.resize(300)
    }

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

    rainbow_btn.addEventListener('click', ()=>{
      fetch('/rainbow',{
        method: "GET"
      })
    })

      const req  = async ()=>{
        let request = await fetch('/updateval');
        let res = await request.text();
        return res;
      }

      req().then((res)=>{
        let strip_state = res.split(',');
        console.log(strip_state)
        state = strip_state[0];
        state_btn.checked = strip_state[0];
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

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request){
    if(LEDStrip.rainbow_state == false){
      LEDStrip.rainbow_state = true;
    }else{
      LEDStrip.rainbow_state = false;
    }
    request->send(200);
  });


  
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  
}
