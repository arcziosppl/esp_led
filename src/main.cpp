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


#define PIN        2
#define NUMPIXELS 18

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);
#define DELAYVAL 200

AsyncWebServer server(80);

const char* ssid = "1283-NET";
const char* password = "*********";


void rainbow(int wait) {
  pixels.clear();
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<pixels.numPixels(); i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show(); 
    delay(wait);  
  }
}

void off_led(){
pixels.clear();
pixels.setPixelColor(NUMPIXELS, pixels.Color(0,0,0));
    pixels.show();
}

void change_color(uint8_t r,uint8_t g, uint8_t b){
 for(int i=0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color( g, r, b ) );
  }

  pixels.show();
}

void on_led(uint8_t g,uint8_t r, uint8_t b){
 for(int i=0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(g,r,b) );
  }

  pixels.show();
}


const char index_html[] PROGMEM = R"rawliteral(
<html>
<head>
<meta charset="UTF-8">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body{
    margin: 0;
    padding: 0;
    background-color: #171F30;
  }

 
  main img{
    height: 40px;
    width: 70px;
    margin-left: 250px;
    margin-top: 10px;
  }

  main{
    width: 280px;
    margin-left: 20px;
  }

  main .color{
    text-align: center;
    font-size: large;
    color: white;
    margin-top: 10px;
  }

  .button {
  border: 1px solid white;;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  margin: 20px;
  cursor: pointer;
}

.button3{
  border: 1px solid white;;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  margin: 20px;
  cursor: pointer;
  width: 230px;
}

.button:hover{
  -webkit-box-shadow: 0px 0px 12px 10px rgba(66, 68, 90, 1);
-moz-box-shadow: 0px 0px 12px 10px rgba(66, 68, 90, 1);
box-shadow: 0px 0px 12px 10px rgba(66, 68, 90, 1);
}

.button3:hover{
  -webkit-box-shadow: 0px 0px 12px 10px rgba(66, 68, 90, 1);
-moz-box-shadow: 0px 0px 12px 10px rgba(66, 68, 90, 1);
box-shadow: 0px 0px 12px 10px rgba(66, 68, 90, 1);
}

.button1, .button2, .button3 {
  background-color: #171F30;
  border-radius: 10px;
}

</style>
<script src="https://cdn.jsdelivr.net/npm/@jaames/iro@5"></script>
<script src="https://requirejs.org/docs/release/2.3.5/minified/require.js"></script>
</head>
<body>
  
  <main>
    <div class="img">
      <a href="https://github.com/arcziosppl"><img src="https://logos-world.net/wp-content/uploads/2020/11/GitHub-Logo.png"></a>
      </div> 

<div class="colorPicker">
  </div>

  <div class="color">
    </div>

    <div class="buttons">
      <button class="button button1">ON</button>
      <button class="button button2">OFF</button>
      <button class="button3">RAINBOW</button>
    </div>
    </main>


    <script>


      var colorPicker = new iro.ColorPicker(".colorPicker", {
        width: 280,
        color: "rgb(255, 0, 0)",
        borderWidth: 1,
        borderColor: "#171F30",
      });
      
      
      colorPicker.on('color:change', function(color) {
        // if the first color changed
        if (color.index === 0) {
          console.log('color 0 changed!');
          // log the color index and hex value
          document.querySelector(".color").innerHTML = "Color: " + color.rgbString;
          console.log(color.rgbString);
           const zapytanie = new XMLHttpRequest();              
              zapytanie.open("GET", "/update?value=" + color.red + "&value2=" + color.green + "&value3=" + color.blue);
              zapytanie.send(); 
        }
      
      });

      var btn_on = document.querySelector(".button1");
        var btn_off = document.querySelector(".button2");
        var btn_set = document.querySelector(".button3");

      btn_on.addEventListener('click', function(){
              const zapytanie = new XMLHttpRequest();              
              zapytanie.open("GET", "/on");
              zapytanie.send();                               
        });
      
        btn_off.addEventListener('click', function(){
          const zapytanie = new XMLHttpRequest();             
              zapytanie.open("GET", "/off");
              zapytanie.send();
        });

        btn_set.addEventListener('click', function(){
          const zapytanie = new XMLHttpRequest();             
              zapytanie.open("GET", "/rainbow");
              zapytanie.send();
        });
      
      
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
  off_led();
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

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){ 
    on_led(0,255,0);                                  
    request->send(200);                                         
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){ 
    off_led(); 
request->send(200);
  });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request){ 
    rainbow(200); 
request->send(200);
  });

server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
 String inputMessage;
  String inputMessage2;
  String inputMessage3;
    if(request->hasParam("value"))
    {
    inputMessage = request->getParam("value")->value();
    inputMessage2 = request->getParam("value2")->value();
    inputMessage3 = request->getParam("value3")->value();

    change_color(inputMessage.toInt(),inputMessage2.toInt(),inputMessage3.toInt());
    }
  

    request->send(200);
  });


  
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  
}
