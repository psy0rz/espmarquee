
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



ESP8266WebServer server(80);
// strip_anim_c<LED_COUNT> strip_anim;



void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
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

// bool smooth=false;
// void handle_set_commands()
// {
//
//     HTTPUpload& upload = server.upload();
//       if (upload.status == UPLOAD_FILE_START)
//       {
//         // Serial.println("Upload started...");
//         // Serial.println(upload.filename);
//         // strip_anim.clear();
//         strip_anim.add_commands_clear();
//       } else if (upload.status == UPLOAD_FILE_WRITE){
//         //if(uploadFile) uploadFile.write(upload.buf, upload.currentSize);
//         // Serial.print("Upload processing bytes:");
//         // Serial.println(upload.currentSize);
//         strip_anim.add_commands(upload.buf, upload.currentSize);
//
//       } else if (upload.status == UPLOAD_FILE_END)
//       {
//         // Serial.println("Upload done, total:");
//         // Serial.print(upload.totalSize);
//         strip_anim.add_commands_done();
//
//         if (!smooth)
//             strip_anim.add_commands_activate(false);
//       }
// }
//
//
// String getContentType(String filename)
// {
//     if(server.hasArg("download")) return "application/octet-stream";
//     else if(filename.endsWith(".htm")) return "text/html";
//     else if(filename.endsWith(".html")) return "text/html";
//     else if(filename.endsWith(".css")) return "text/css";
//     else if(filename.endsWith(".js")) return "application/javascript";
//     else if(filename.endsWith(".png")) return "image/png";
//     else if(filename.endsWith(".gif")) return "image/gif";
//     else if(filename.endsWith(".jpg")) return "image/jpeg";
//     else if(filename.endsWith(".ico")) return "image/x-icon";
//     else if(filename.endsWith(".xml")) return "text/xml";
//     else if(filename.endsWith(".pdf")) return "application/x-pdf";
//     else if(filename.endsWith(".zip")) return "application/x-zip";
//     else if(filename.endsWith(".gz")) return "application/x-gzip";
//     return "text/plain";
// }

//
// bool handleFileRead(String path)
// {
//     if(path.endsWith("/"))
//     {
//         path += "index.html";
//     }
//
//     // Serial.println("handleFileRead: " + path);
//
//     String contentType = getContentType(path);
//
//     // path=hashed_gz_path(path);
//     // Serial.println("hashed file: " + path);
//
//     if(SPIFFS.exists(path))
//     {
//         File file = SPIFFS.open(path, "r");
//         server.sendHeader("Cache-Control","max-age=86400");
//         size_t sent = server.streamFile(file, contentType);
//         file.close();
//         // server.client().stop();
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }
//
//
// void return_ok(const __FlashStringHelper* flashString)
// {
//     server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
//     server.send(200, "text/plain", String(flashString));
// }
//
// void return_ok()
// {
//     server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
//     server.send(200, "text/plain", "");
// }
//
// void return_error(const __FlashStringHelper* flashString)
// {
//     server.send(500, "text/plain", String(flashString));
// }
//

//send current strip_anim values to hardware
// void send_leds()
// {
//                 neopixel_strip->SetPixelColor(l, RgbColor(
//                     strip_anim.led_anim.led_level[l].r,
//                     strip_anim.led_anim.led_level[l].g,
//                     strip_anim.led_anim.led_level[l].b));
//             neopixel_strip->Show();
//         }
// }

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

  server.on("/", HTTP_GET, [](){
    server.send(404, "text/plain", "FileNotFound");
  });


  server.begin();

  // WiFi.begin();
  // WiFi.mode(WIFI_AP_STA);
  if ( WiFi.softAP("lichtkrant1", "toolboxdinges"))
  Serial.print("wifi ok\n");
  else
  Serial.print("wifi failed\n");


  Serial.println("boot complete");
  Serial.println(ESP.getFreeHeap());
}


unsigned long last_micros=0;
int last_wifi_status=-1;
unsigned long last_check=0;

void periodic_checks()
{
  wifi_config();


}




class ProgressiveScroller
{
public:
  int charnr=0;
  int xoffset=0;
  int space=0;
  String text="Welkom bij TOOLBOX   ";
  RgbColor color;


  void step()
  {
    //stop text as this point
    for(int i=0;i<8; i++)
    strip.SetPixelColor( topo.Map(3,i), RgbColor(0,0,0));


    // if (space){
    //   //draw empty space
    //   for(int i=0;i<8; i++)
    //     strip.SetPixelColor( topo.Map(WIDTH-2,i), RgbColor(0,0,0));
    //
    //   space--;
    // }
    // else
    {
      //draw one pixelline of a character
      color=RgbColor( (charnr%6)*15, ((charnr+1)%6)*15, ((charnr+2)%6)*15) ;
      // color=RgbColor( 0,0,255) ;

      if (drawCharLine(text[charnr], WIDTH-1, xoffset, color))
      {
        //character complete
        xoffset=0;
        charnr=charnr+1;
        if (charnr>=text.length())
        {
          //scrolled through all text
          charnr=0;
        }
      }
      else
      {
        xoffset=xoffset+1;
      }
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


  // //obsolete for now, adafruit fonts too big for this marquee
  // bool drawCharLine(unsigned char c, int16_t xoffset, RgbColor color)
  // {
  //     int x=WIDTH-2;
  //
  //     c -= (uint8_t)pgm_read_byte(&gfxFont->first);
  //     GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
  //     uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
  //
  //     uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
  //     uint8_t  w  = pgm_read_byte(&glyph->width),
  //              h  = pgm_read_byte(&glyph->height);
  //     int8_t   xo = pgm_read_byte(&glyph->xOffset),
  //              yo = pgm_read_byte(&glyph->yOffset);
  //     uint8_t  xx, yy, bits = 0, bit = 0;
  //     int16_t  xo16 = 0, yo16 = 0;
  //
  //     // if(size > 1) {
  //     //     xo16 = xo;
  //     //     yo16 = yo;
  //     // }
  //
  //     for(yy=0; yy<h; yy++) {
  //         for(xx=0; xx<w; xx++)
  //         {
  //             if(!(bit++ & 7)) {
  //                 bits = pgm_read_byte(&bitmap[bo++]);
  //             }
  //
  //             if (xx==xoffset)
  //             {
  //               int y=yo+yy+HEIGHT-1;  //font starts at the bottom and works up
  //
  //               if (y>=0 && y<HEIGHT)
  //               {
  //                 if(bits & 0x80) {
  //                   //pixel on
  //                   // strip.SetPixelColor( topo.Map(x+xo+xx,y+yo+yy), color);
  //                   strip.SetPixelColor( topo.Map(x,y), color);
  //                 }
  //                 else
  //                 {
  //                   //off
  //                   // strip.SetPixelColor( topo.Map(x+xo+xx,y+yo+yy), RgbColor(0,0,0));
  //                   strip.SetPixelColor( topo.Map(x,y), RgbColor(0,0,0));
  //
  //                 }
  //               }
  //             }
  //             bits <<= 1;
  //         }
  //     }
  //
  //     return(xoffset+1>=w);
  //   }
};






ProgressiveScroller scroller;



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
