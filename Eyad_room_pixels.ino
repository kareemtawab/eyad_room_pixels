#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include <ESP8266WiFiMulti.h>
#include <DMDESP.h>
#include <fonts/EMSans8x16.h>
#include <ESP8266mDNS.h>

#define DISPLAYS_WIDE 1
#define DISPLAYS_HIGH 1
#define DISPLAY_BRIGHTNESS 768 // (768)
#define DISP_FONT EMSans8x16

ESP8266WebServer server(80);
ESP8266WiFiMulti wifiMulti;
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);

IPAddress local_IP(192, 168, 1, 159);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 167);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

String MAIN_page() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Eyad's Dot Matrix</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: table; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += "p {font-size: 20px;color: #888;margin-bottom: 14px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Eyad's Dot Matrix</h1>\n";
  ptr += "<h2>Scrolling Text</h2>\n";

  ptr += "<form action=/action_page>\n<table>\n<tr>\n";
  ptr += "<td align=right>Text (1-32):</td>\n";
  ptr += "<td align=left><input type=text name=text value=Eyad></td>\n";
  ptr += "</tr>\n<tr>";
  ptr += "<td align=right>Frame delay (20-140):</td>\n";
  ptr += "<td align=left><input type=text name=speed value=75></td>\n";
  ptr += "</tr>\n<tr>";
  ptr += "<td align=right>Repeat (1-10):</td>\n";
  ptr += "<td align=left><input type=text name=repeat value=2></td>\n";
  ptr += "</tr>\n</table>\n";
  ptr += "<input type=submit value=START>\n";
  ptr += "</form>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
String DONE_page() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Eyad's Dot Matrix</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += "p {font-size: 20px;color: #888;margin-bottom: 14px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Eyad's Dot Matrix</h1>\n";
  ptr += "<h2>Scrolling Text</h2>\n";
  ptr += "<h3><a href='/'>Done! Click here to restart.</a></h3>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
int blinkfrequency[512];
unsigned long startblink[512];
bool ledstate[512];
int systemmode = 0;
String scrolltext;
int scrolldelay;
int repeater;
int repeated;
bool scrollingisdone = false;

void drawScrollText(int y, int scrolldelay) {
  static uint32_t pM;
  static uint32_t x = 0;
  int width = Disp.width();
  int fullScroll = Disp.textWidth(scrolltext) + width;

  if ((millis() - pM) > scrolldelay) {
    pM = millis();
    if (x < fullScroll) {
      ++x;
    }
    else {
      x = 0;
      repeated++;
      scrollingisdone = true;
      return;
    }
    Disp.clear();
    Disp.loop();
    Disp.drawText(width - x, y, scrolltext);
  }
}

void handleRoot() {
  server.send(200, "text/html", MAIN_page()); //Send web page
}

void handleForm() {
  scrolltext = server.arg("text");
  String a = server.arg("speed");
  scrolldelay = a.toInt();
  scrolldelay = constrain(scrolldelay, 20, 140);
  String b = server.arg("repeat");
  repeater = b.toInt();
  repeater = constrain(repeater, 1, 10);
  //handleRoot();
  server.send(200, "text/html", DONE_page()); //Send web page
  systemmode = 1;
  //String goback = "<a href='/'> Go Back </a>";
}

void setup() {
  scrolltext.reserve(32);
  Disp.setBrightness(DISPLAY_BRIGHTNESS); //maximum is 1024
  Disp.start();
  Disp.setFont(DISP_FONT);
  Disp.drawText(0, 0, ".con");
  Disp.loop();
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  wifiMulti.addAP("WAW20", "219adeltawab");
  wifiMulti.addAP("WAW20_HK1", "219adeltawab");
  wifiMulti.addAP("WAW20_HK2", "219adeltawab");
  wifiMulti.addAP("WAW20_HK3", "219adeltawab");
  wifiMulti.addAP("HK SG Note 9", "07081989");
  wifiMulti.run();
  WiFi.hostname("EyadDMD");
  MDNS.begin("EyadDMD");
  server.on("/", handleRoot);
  server.on("/action_page", handleForm);
  ElegantOTA.begin(&server);
  server.begin();
  for (int i = 0; i < 512; i++) {
    blinkfrequency[i] = random(400, 600);
  }
  for (int i = 0; i < 512; i++) {
    startblink[i] = 0;
  }
}

void loop() {
  wifiMulti.run();
  server.handleClient();
  switch (systemmode) {
    case 0:
      for (int i = 0; i < 512; i++) {
        if (millis() >= startblink[i] + blinkfrequency[i]) {
          ledstate[i] = !ledstate[i];
          startblink[i] += blinkfrequency[i];
        }
      }
      for (int m = 0; m < 16; m++) {
        for (int i = 0; i < 32; i++) {
          Disp.setPixel(i, m, ledstate[i + 32 * m]);
        }
      }
      break;
    case 1:
      if (scrollingisdone && repeated >= repeater) {
        repeated = 0;
        systemmode = 0;
      }
      else {
        drawScrollText(0, scrolldelay);
      }
      break;
  }
  Disp.loop();
}
