
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "FS.h"
#include <NeoPixelBus.h>


#include <gfxfont.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/Picopixel.h>

// const GFXfont *gfxFont=&FreeMono9pt7b;
// const GFXfont *gfxFont=&FreeSans9pt7b;
const GFXfont *gfxFont=&FreeSerif9pt7b;
// const GFXfont *gfxFont=&Org_01;
// const GFXfont *gfxFont=&Picopixel;


//topology
#define LED_COUNT 300
#define WIDTH 38
#define HEIGHT 8

NeoTopology <RowMajor180Layout> topo(WIDTH,HEIGHT);
NeoPixelBus<NEOPIXEL_CONFIG> strip(LED_COUNT);



// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif



ESP8266WebServer server(80);
// strip_anim_c<LED_COUNT> strip_anim;



// void handleRoot() {
//     server.send(200, "text/plain", "hello from esp8266!");
// }
//

void wifi_config()
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



    // // load default animation from flash
    // File fh=SPIFFS.open("commands.dat", "r+");
    // if (fh)
    // {
    //     strip_anim.add_commands_clear();
    //
    //     while(fh.available())
    //     {
    //       // Serial.println("byteee");
    //         uint8_t b;
    //         b=fh.read();
    //         strip_anim.add_commands(&b, 1);
    //     }
    //     fh.close();
    //     strip_anim.add_commands_activate(false);
    // }


    // server.on("/set_commands", HTTP_POST, [](){
    //     smooth=false;
    //     return_ok();
    // }, handle_set_commands);
    //
    // server.on("/set_commands", HTTP_OPTIONS, [](){
    //     return_ok();
    // });
    //
    // server.on("/set_commands_smooth", HTTP_POST, [](){
    //     smooth=true;
    //     return_ok();
    // }, handle_set_commands);
    //
    // server.on("/set_commands_smooth", HTTP_OPTIONS, [](){
    //     return_ok();
    // });

    //power on/off ATX supply. when booting we always turn the ATX power on. (you can turn it off via the webgui)
    // pinMode(PIN_POWER_ON, OUTPUT);
    // digitalWrite(PIN_POWER_ON, 1);
    // server.on("/off", HTTP_GET, [](){
    //     digitalWrite(PIN_POWER_ON, 0);
    //     return_ok();
    // });
    //
    // server.on("/on", HTTP_GET, [](){
    //     digitalWrite(PIN_POWER_ON, 1);
    //     return_ok();
    // });

    //save current command string to SPIFFS
    // server.on("/save", HTTP_GET, [](){
    //     // server.send(404, "text/plain", "FileNotFound");
    //     File fh=SPIFFS.open("commands.dat", "w");
    //     if (!fh)
    //     {
    //         return_error(F("Error creating save file."));
    //     }
    //     else
    //     {
    //         commands_t::iterator i;
    //         i=strip_anim.commands.begin();
    //         while(i!=strip_anim.commands.end())
    //         {
    //             if (fh.write(*i)!=1)
    //             {
    //                 fh.close();
    //                 return_error(F("Error while saving."));
    //                 return;
    //             }
    //             i++;
    //         }
    //         fh.close();
    //         return_ok(F("Saved current animation as default."));
    //     }
    // });
    //
    // server.onNotFound([](){
    //     if(!handleFileRead(server.uri()))
    //     {
    //         server.send(404, "text/plain", "FileNotFound");
    //         // server.client().stop();
    //     }
    // });
    //
    // server.begin();


    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");


    // No authentication by default
    // ArduinoOTA.setPassword((const char *)"123");

//     ArduinoOTA.onStart([]() {
//         Serial.println("OTA: Start");
//         SPIFFS.end(); //important
//         strip_anim.led_anim.clear(CRGB(0,255,0)); //still safe
//         send_leds();
//     });
//     ArduinoOTA.onEnd([]() {
//         Serial.println("\nOTA: End");
//         //"dangerous": if you reset during flash you have to reflash via serial
//         //so dont touch device until restart is complete
//         strip_anim.led_anim.clear(CRGB(255,0,0));
//         send_leds();
//         Serial.println("\nOTA: DO NOT RESET OR POWER OFF UNTIL BOOT+FLASH IS COMPLETE.");
//         delay(100);
// #ifdef NEOPIXEL_CONFIG
//         delete neopixel_strip; //imporatant, otherwise conflicts with flashing i think
//         neopixel_strip=NULL;
// #endif
//         ESP.reset();
//     });
//     ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//
//         Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//         //getting more "dangerous"
//         strip_anim.led_anim.set((progress*LED_COUNT)/total, CRGB(255,255,0));
//         send_leds();
//     });
//     ArduinoOTA.onError([](ota_error_t error) {
//         Serial.printf("Error[%u]: ", error);
//         if (error == OTA_AUTH_ERROR) Serial.println("OTA: Auth Failed");
//         else if (error == OTA_BEGIN_ERROR) Serial.println("OTA: Begin Failed");
//         else if (error == OTA_CONNECT_ERROR) Serial.println("OTA: Connect Failed");
//         else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA: Receive Failed");
//         else if (error == OTA_END_ERROR) Serial.println("OTA: End Failed");
//     });
//     ArduinoOTA.begin();

// for(int i=0;i<8; i++)
//   strip.SetPixelColor( topo.Map(i+o,0), RgbColor(10,0,0));
//
//   for(int i=0;i<8; i++)
//     strip.SetPixelColor( topo.Map(i+o,i), RgbColor(0,10,0));
//
  // for(int i=0;i<8; i++)
  //   strip.SetPixelColor( topo.Map(o,i), RgbColor(0,0,10));
  //
  //   strip.SetPixelColor( topo.Map(8,0), RgbColor(10,0,0));
  //   strip.SetPixelColor( topo.Map(8,1), RgbColor(0,10,0));
  //   strip.SetPixelColor( topo.Map(8,2), RgbColor(0,0,10));


    Serial.println("boot complete");
}


unsigned long last_micros=0;
int last_wifi_status=-1;
unsigned long last_check=0;

void periodic_checks()
{
  if (last_wifi_status!=WL_CONNECTED)
    wifi_config();
}




class ProgressiveScroller
{
  public:
    int charnr=0;
    int xoffset=0;
    int space=0;
    String text="toolbox      ";
    RgbColor color;


    void step()
    {
      //stop text as this point
      for(int i=0;i<8; i++)
        strip.SetPixelColor( topo.Map(3,i), RgbColor(0,0,0));


      if (space){
        //draw empty space
        for(int i=0;i<8; i++)
          strip.SetPixelColor( topo.Map(WIDTH-2,i), RgbColor(0,0,0));

        space--;
      }
      else
      {
        //draw one pixelline of a character
        color=RgbColor( (charnr%6)*64, ((charnr+1)%6)*64, ((charnr+2)%6)*64) ;

        if (drawCharLine(text[charnr], xoffset, color))
        {
          //character complete
          xoffset=0;
          charnr=charnr+1;
          space=2;
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
    }

    bool drawCharLine(unsigned char c, int16_t xoffset, RgbColor color)
    {
      int x=WIDTH-2;
      // int y=HEIGHT-1;
      // int y=0;
  //
  //
  //
  //
  //     void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c,
  // uint16_t color, uint16_t bg, uint8_t size) {


        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
        uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);
        uint8_t  xx, yy, bits = 0, bit = 0;
        int16_t  xo16 = 0, yo16 = 0;

        // if(size > 1) {
        //     xo16 = xo;
        //     yo16 = yo;
        // }

        for(yy=0; yy<h; yy++) {
            for(xx=0; xx<w; xx++)
            {
                if(!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }

                if (xx==xoffset)
                {
                  int y=yo+yy+HEIGHT-1;  //font starts at the bottom and works up

                  if (y>=0 && y<HEIGHT)
                  {
                    if(bits & 0x80) {
                      //pixel on
                      // strip.SetPixelColor( topo.Map(x+xo+xx,y+yo+yy), color);
                      strip.SetPixelColor( topo.Map(x,y), color);
                    }
                    else
                    {
                      //off
                      // strip.SetPixelColor( topo.Map(x+xo+xx,y+yo+yy), RgbColor(0,0,0));
                      strip.SetPixelColor( topo.Map(x,y), RgbColor(0,0,0));

                    }
                  }
                }
                bits <<= 1;
            }
        }

        return(xoffset+1>=w);
      }
};






ProgressiveScroller scroller;







void loop(void){
    // Serial.println("lup");
    server.handleClient();


    if (WiFi.status() != last_wifi_status)
    {
        last_wifi_status=WiFi.status();
        if (last_wifi_status==WL_CONNECTED)
        {
            Serial.print("Wifi connected IP address: ");
            Serial.print(WiFi.localIP());
            Serial.print(", strength: ");
            Serial.println(WiFi.RSSI());
        }
        else
        {
            Serial.print("Wifi disconnected, status:");
            Serial.println(last_wifi_status);
        }

    }


    ArduinoOTA.handle();

    //do periodic checks ever 10s
    if ((millis()-last_check)>10000)
    {
      periodic_checks();
      last_check=millis();
    }


    //profiling
    // Serial.print("microseconds used: ");
    // Serial.println(micros()-last_micros);
    // with the last tests i did it was usually lower than 2000uS (for both strips)





    // strip.ClearTo(RgbColor(0,0,0));
    scroller.step();
    strip.Show();
    // strip.RotateLeft(1);

    //cap at 60 fps
    // while (micros()-last_micros < 16666);
    while (micros()-last_micros < 64000);
    last_micros=micros();

}