
#include <Arduino.h>
#include <ESP8266WiFi.h>
// #include <WiFiClient.h>
#include <ESP8266WebServer.h>
// #include <ESP8266mDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>

#include "FS.h"
#include <NeoPixelBus.h>


#include <gfxfont.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/Picopixel.h>

#include <font.h>

// const GFXfont *gfxFont=&FreeMono9pt7b;
// const GFXfont *gfxFont=&FreeSans9pt7b;
// const GFXfont *gfxFont=&FreeSerif9pt7b;
// const GFXfont *gfxFont=&Org_01;
// const GFXfont *gfxFont=&Picopixel;


//topology
#define WIDTH 38
#define HEIGHT 8
// #define WIDTH 144*2
// #define HEIGHT 9
#define LED_COUNT WIDTH*HEIGHT
#define FPS 20

NeoTopology <RowMajor180Layout> topo(WIDTH,HEIGHT);
// NeoPixelBus<NEOPIXEL_CONFIG> strip(LED_COUNT);
NeoPixelBus<NeoGrbFeature,Neo800KbpsMethod> strip(LED_COUNT);



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

public:
  int version=0;


  //reset current position and color to beginning
  void reset()
  {
    charnr=0;
    xoffset=0;
    color=RgbColor(255,0,0);
  }

  void setText(const String & new_text)
  {
    version++;
    reset();
    strip.ClearTo(0);
    text=new_text;
  }

  const String & getText()
  {
    return(text);
  }

  void step()
  {
    //stop text as this point
    for(int i=0;i<8; i++)
    strip.SetPixelColor( topo.Map(3,i), RgbColor(0,0,0));

    //draw one pixelline of a character
    color=RgbColor( (charnr%6)*15, ((charnr+1)%6)*15, ((charnr+2)%6)*15) ;
    // color=RgbColor( 0,0,255) ;

    if (drawCharLine(text[charnr], WIDTH-1, xoffset, color))
    {
      //character complete
      xoffset=0;
      charnr=charnr+1;

      //filter garbage (newlines and stuff)
      while(charnr<text.length() && text[charnr]<ASCII_OFFSET) charnr++;

      if (charnr>=text.length())
      {
        //scrolled through all text
        reset();
      }
    }
    else
    {
      xoffset=xoffset+1;
    }

    strip.ShiftRight(1);
    strip.Show();
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
      strip.SetPixelColor( topo.Map(x,y), color);
      else
      strip.SetPixelColor( topo.Map(x,y), 0);
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
  html=F("\
  <!doctype html>\
  <html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head><body>\
  <form method='post'>\
  <input type='submit' value='send'><br>\
  <textarea autofocus name='txt' rows=40 cols=40>");

  html=html+scroller.getText();

  html=html+F("</textarea>\
  </form>\
  </body>");
  server.send(200, "text/html",html);


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

void setup(void){
  Serial.begin(115200);

  delay(100);
  Serial.println();
  Serial.println("Marquee 1.0 booting...");

  // Serial.println(ESP.getSketchSize());
  // Serial.println(ESP.getFreeSketchSpace());

  strip.Begin();


  if (!SPIFFS.begin())
  Serial.println("SPIFFS: error while mounting");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handleRoot);


  server.begin();


  if ( WiFi.softAP("lichtkrant1", "toolboxdinges"))
  Serial.print("wifi ok\n");
  else
  Serial.print("wifi failed\n");


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


unsigned long last_micros=0;
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

unsigned long idle_us=0;
void loop(void){
  // Serial.println("lup");
  server.handleClient();




  //ArduinoOTA.handle();

  //do periodic checks ever 10s
  if ((millis()-last_check)>1000)
  {
    Serial.printf("Load: %d\n", 100- ((idle_us/10)/(millis()-last_check) ));
    idle_us=0;

    periodic_checks();
    last_check=millis();
  }


  //profiling
  // Serial.print("microseconds used: ");
  // Serial.println(micros()-last_micros);
  // with the last tests i did it was usually lower than 2000uS (for both strips)





  // strip.ClearTo(RgbColor(0,0,0));
  // while(!strip.CanShow()) yield();
  //strip.Show();
  // strip.RotateLeft(1);

  //cap at 60 fps
  // while (micros()-last_micros < 16666);
  int count=0;

  unsigned long delta=micros()-last_micros;
  // Serial.println(delta);
  if (delta < (1000000/FPS))
  {
    unsigned long time_left= (1000000/FPS)  - ( delta);
    idle_us=idle_us+time_left;
    // delayMicroseconds(time_left);
    delay(time_left/1000);
  }
  //    while (micros()-last_micros < 1500*0.03*2) count++;
  // Serial.println(count);
  last_micros=micros();
  scroller.step();

}
