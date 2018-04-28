
#include <Arduino.h>
#include <ESP8266WiFi.h>
// #include <WiFiClient.h>
#include <ESP8266WebServer.h>
// #include <ESP8266mDNS.h>
// #include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "FS.h"
#include <NeoPixelBus.h>
#include <internal/HtmlColor.h>

#include <font.h>
// #include <txbuf.hpp>

//topology
#define WIDTH 38
#define HEIGHT 8
// #define WIDTH 144*2
// #define HEIGHT 9
#define LED_COUNT WIDTH*HEIGHT

NeoTopology <RowMajor180Layout> topo(WIDTH,HEIGHT);
// NeoPixelBus<NEOPIXEL_CONFIG> strip(LED_COUNT);
NeoPixelBus<NeoGrbFeature,Neo800KbpsMethod> *strip=new NeoPixelBus<NeoGrbFeature,Neo800KbpsMethod>(LED_COUNT);



// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

class ProgressiveScroller
{
private:
  int charnr=0;
  int xoffset=0;
  String text=".";
  RgbColor color;
  RgbColor bgcolor;
  char current_char;
  unsigned long last_micros=0;
  int fps=25;

public:
  int version=0;


  //reset current position and color to beginning
  void reset()
  {
    charnr=0;
    xoffset=0;
    color=RgbColor(255,0,0);
    bgcolor=0;
    fps=25;
    gotoNextChar();
  }

  void setText(const String & new_text)
  {
    version++;
    text=new_text;
    reset();
    strip->ClearTo(0);
  }

  const String & getText()
  {
    return(text);
  }

  //get next character to be displayed and parse macros
  void gotoNextChar()
  {

    //filter garbage (newline) and parse escape chars
    while(charnr<text.length() && ( text[charnr]<' ' || text[charnr]=='['))
    {
      //start of macro
      if (text[charnr]=='[')
      {
        int close=text.indexOf(']', charnr);
        //found end?
        if (close>=charnr+2)
        {
          char cmd=text[charnr+1];

          String params=text.substring(charnr+2,close);
          switch(cmd)
          {

            //special characters
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
              current_char=cmd-'0';
              charnr=close+1;
              return;
              break;
            }


            // foreground color
            case '#':
            {
              HtmlColor htmlcolor;
              htmlcolor.Parse<HtmlColorNames>(text.substring(charnr+1,close));
              color=htmlcolor;
              break;
            }
            // background color
            case 'B':
            {
              HtmlColor htmlcolor;
              htmlcolor.Parse<HtmlColorNames>(params);
              bgcolor=htmlcolor;
              break;
            }
            // speed
            case 'S':
            {
              fps=params.toInt();
              break;
            }
            default:
            {
              break;
            }
          }
        }
        charnr=close;
      }
      charnr++;
    }

    //no valid chars left?
    if (charnr>=text.length())
    {
      current_char=' ';
      reset();
    }
    else
    {
      current_char=text[charnr];
      charnr=charnr+1;
    }
  }


  void fpswait()
  {
    unsigned long delta=micros()-last_micros;
    // Serial.println(delta);
    if (delta < (1000000/fps))
    {
      unsigned long time_left= (1000000/fps)  - ( delta);
      // idle_us=idle_us+time_left;
      // delayMicroseconds(time_left);
      delay(time_left/1000);
    }
    //    while (micros()-last_micros < 1500*0.03*2) count++;
    // Serial.println(count);
    last_micros=micros();
  }

  //step one pixel
  void step()
  {
    fpswait();

    //erase text as this point
    for(int i=0;i<8; i++)
    strip->SetPixelColor( topo.Map(3,i), RgbColor(0,0,0));

    //draw one pixelline of a character

    if (drawCharLine(current_char, WIDTH-1, xoffset, color))
    {
      //character complete, go to next character
      xoffset=0;
      gotoNextChar();

    }
    else
    {
      xoffset=xoffset+1;
    }

    strip->ShiftRight(1);
    strip->Show();
  }


  //draws one vertical fontline at the x position
  bool drawCharLine(unsigned char c, int16_t x, int16_t xoffset, RgbColor color)
  {
    const uint8_t *charbase = FontData + (( c - ASCII_OFFSET)* FONT_WIDTH );


    char pixels;

    //at the end of this fonts data yet?
    if (xoffset<FONT_WIDTH)
    pixels=pgm_read_byte_near(charbase+xoffset);
    else
    pixels=0;

    for (int16_t y=0; y<FONT_HEIGHT; y++)
    {
      if (pixels & (1<<(FONT_HEIGHT-y)))
      strip->SetPixelColor( topo.Map(x,y), color);
      else
      strip->SetPixelColor( topo.Map(x,y), bgcolor);
    }

    //letter + space complete?
    if (xoffset>=FONT_WIDTH)
    return true;
    else
    return false;
  }


};


ProgressiveScroller scroller;
ESP8266WebServer server(80);
// strip_anim_c<LED_COUNT> strip_anim;



void handleRoot() {
  if (server.hasArg(F("txt")))
  {
    scroller.setText(server.arg(F("txt")));
    scroller.reset();
  }


  String html="";
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200);
  server.sendContent(F("\
<!doctype html>\
<html><head>\
<style>\
p {\
display: inline-block;\
border-style: solid;\
padding: 0.5em;\
margin: 0.1em;\
min-width: 2em;\
text-align: center;\
}\
</style>\
<script>\
  function typeInTextarea(el, newText) {\
  var start = el.selectionStart;\
  var end = el.selectionEnd;\
  var text = el.value;\
  var before = text.substring(0, start);\
  var after  = text.substring(end, text.length);\
  el.value = (before + newText + after);\
  el.selectionStart = el.selectionEnd = start + newText.length;\
  el.focus();\
  };\
 function i(text) {\
  typeInTextarea(document.getElementById('txt'), text);\
};\
</script>"));

  server.sendContent(F("\
<meta name='viewport' content='width=device-width, initial-scale=1'>\
</head><body>\
<p onclick='i(\"[#ff0000]\")' style='background-color: #ff0000'>.</p>\
<p onclick='i(\"[#00ff00]\")' style='background-color: #00ff00'>.</p>\
<p onclick='i(\"[#0000ff]\")' style='background-color: #0000ff'>.</p>\
<p onclick='i(\"[#ffff00]\")' style='background-color: #ffff00'>.</p>\
<p onclick='i(\"[#00ffff]\")' style='background-color: #00ffff'>.</p>\
<p onclick='i(\"[#ff00ff]\")' style='background-color: #ff00ff'>.</p>\
<p onclick='i(\"[#ffffff]\")' style='background-color: #ffffff'>.</p>\
<p onclick='i(\"[S20]\")' >20pps</p>\
<p onclick='i(\"[S25]\")' >25pps</p>\
<p onclick='i(\"[S30]\")' >30pps</p>\
<p onclick='i(\"[#ffffff]T[#ff0000][1][2][#ffffff]LB[#ff0000][0][#ffffff]X\")' >logo</p>\
<form method='post'>\
<p><input type='submit' value='send'></p><br>\
<textarea autofocus id='txt' name='txt' rows=40 cols=40>"));

  server.sendContent(scroller.getText());

  server.sendContent(F("</textarea>\
  </form>\
  </body>"));
  // server.send(200, "text/html",html);
  server.sendContent("");


}


void wifi_config()
{
  Serial.printf("Wifi stations connected = %d\n", WiFi.softAPgetStationNum());
  if (WiFi.status()==WL_CONNECTED)
  {
    Serial.print("Wifi connected IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(", strength: ");
    Serial.println(WiFi.RSSI());
  }
  else
  {
    File wifi_config_fh = SPIFFS.open("/wifi.txt", "r");
    if (!wifi_config_fh) {
      Serial.println("cannot load wifi config.");
    }
    else
    {
      String ssid=wifi_config_fh.readStringUntil('\n');
      String password=wifi_config_fh.readStringUntil('\n');
      wifi_config_fh.close();

      Serial.print("Wifi connecting to: ");
      Serial.println(ssid.c_str());
      WiFi.setAutoReconnect(true);
      WiFi.begin(ssid.c_str(), password.c_str());
    }
  }
}

int saved_version=0;

void progress(int percentage, RgbColor color)
{
  for (int i=0; i< percentage*50/100; i++)
  strip->SetPixelColor(i, color);
  strip->Show();

}

void setup(void){
  Serial.begin(115200);

  delay(100);
  Serial.println();
  Serial.println("Marquee 1.0 booting...");

  // Serial.println(ESP.getSketchSize());
  // Serial.println(ESP.getFreeSketchSpace());

  strip->Begin();


  if (!SPIFFS.begin())
  Serial.println("SPIFFS: error while mounting");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handleRoot);


  server.begin();


  if ( WiFi.softAP("lichtkrant1", "toolboxdinges"))
  Serial.print("wifi ok\n");
  else
  Serial.print("wifi failed\n");

  // OTA
  ArduinoOTA.onStart([]() {
    Serial.println("OTA: Start");
    SPIFFS.end(); //important
    strip->ClearTo(0);
    progress(100,RgbColor(0,255,0));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA: End");
    //"dangerous": if you reset during flash you have to reflash via serial
    //so dont touch device until restart is complete
    progress(100,RgbColor(255,0,0));
    Serial.println("\nOTA: DO NOT RESET OR POWER OFF UNTIL BOOT+FLASH IS COMPLETE.");
    delay(100);

    delete strip; //imporatant, otherwise conflicts with flashing i think

    ESP.reset();
  });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {

    Serial.printf("Progress: %u%%\r", (done * 100 / total));
    //getting more "dangerous"
    progress(done*100/total, RgbColor(255,255,0));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA: Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA: Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA: Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA: Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA: End Failed");
  });
  ArduinoOTA.begin();



  // load text
  File fh = SPIFFS.open("/text.txt", "r");
  if (fh)
  {
    scroller.setText(fh.readString());
    saved_version=scroller.version;
    fh.close();
  }

  Serial.println("boot complete");
  Serial.println(ESP.getFreeHeap());
}


int last_wifi_status=-1;
unsigned long last_check=0;



void periodic_checks()
{
  wifi_config();

  if (saved_version != scroller.version)
  {
    Serial.println("Saving text to flash");
    File fh = SPIFFS.open("/text.txt", "w");
    fh.print(scroller.getText());
    saved_version=scroller.version;
  }


}

// unsigned long idle_us=0;
void loop(void){
  // Serial.println("lup");
  ArduinoOTA.handle();
  server.handleClient();

  //do periodic checks ever 10s
  if ((millis()-last_check)>10000)
  {

    periodic_checks();
    last_check=millis();
  }

  scroller.step();

}
