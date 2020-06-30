#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define TelloAddress "192.168.10.1"
#define TelloCommandPort 8889

enum TelloMode {
  tm_none = 0, tm_takeoff, tm_land, tm_reset, tm_max
};

const char *tm_text[] = {
  "none",
  "takeoff",
  "land",
  "reset",
  "???"
};

class TelloInfo {
  public:
    char ssid[128];
    char address[16];
    int commandPortNo;
    char status[80];
    bool connected;
    char message[256];
    int mode;
    double vbat;      // M5StickC VBat

    TelloInfo() {
      ssid[0] = '\0';
      address[0] = '\0';
      status[0] = '\0';
      message[0] = '\0';
      connected = false;
      mode = tm_none;
      vbat = 0.0;
    };
    void next() {
      mode = (mode + 1) % tm_max;
    }
};

TelloInfo Tello;

void error()
{
  while(1) {    
  }
}

//Status: xxxx
//SSID: xxxx
//Tello: xxxx:yyyy
//msg: xxx
//mode: xxx
//bat: %.1f (%d%%)

void updateStatus(bool refresh, const char *status)
{
  if (refresh) {
    M5.Lcd.fillScreen(BLACK);
  }
  if (status) {
    strcpy(Tello.status, status);
  }
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Status: %s ", Tello.status);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.printf("SSID: %s ", Tello.ssid);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("Tello: %s:%d ", Tello.address, Tello.commandPortNo);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.printf("msg: %s ", Tello.message);
  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("mode: %s ", tm_text[Tello.mode]);
  Tello.vbat = M5.Axp.GetVbatData() * 1.1 / 1000;
  int charge = int8_t((Tello.vbat - 3.0) / 1.2 * 100);
  if(charge > 100){
    charge = 100;
  }else if(charge < 0){
    charge = 0;
  }
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.printf("bat: %.2fV (%3d%%) ", Tello.vbat, charge);
}

bool wifi_connect(const char *ssid, const char *passwd)
{
  updateStatus(false, "connecting...");
  WiFi.begin(ssid, passwd);
  updateStatus(false, "waiting...");
  Tello.connected = false;
  for (int count = 0; count < 30; count ++) {
    if (WiFi.status() == WL_CONNECTED) {
      updateStatus(true, "connected");
      Tello.connected = true;
      break;
    }
    delay(500);
    M5.Lcd.print(".");
  }
  if (!Tello.connected) {
    updateStatus(true, "Timed out");
    return false;
  }
  return true;
}

bool connect_tello()
{
  bool found = false;
  int num = WiFi.scanNetworks();
  updateStatus(true, "start scan");
  for (int i = 0; i < num; i ++) {
    const char *_ssid = WiFi.SSID(i).c_str();
    M5.Lcd.print(_ssid);
    if (!strncmp(_ssid, "TELLO-", 6)) {
      strcpy(Tello.ssid, _ssid);
      found = true;
      break;
    }
  }
  if (!found) {
    updateStatus(true, "Tello not found");
    return false;
  }
  return wifi_connect(Tello.ssid, "");
}

WiFiUDP Udp;

void send(const char *buf)
{
  int len = strlen(buf);
  if (len > sizeof(Tello.message) - 1) {
    updateStatus(true, "too long message");
    return;
  }
  strcpy(Tello.message, buf);
  char c = Tello.message[len - 1];
  if (c == '\r' || c == '\n') {
    Tello.message[-- len] = '\0';
  }
  if (len == 0) {
    updateStatus(true, "blank message");
    return;
  }
  updateStatus(false, "sent");
  if (Tello.connected) {
    Udp.beginPacket(Tello.address, Tello.commandPortNo);
    Udp.write((const uint8_t *) Tello.message, len);
    Udp.endPacket();
  }
}

void setup()
{
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  Serial2.begin(115200, SERIAL_8N1, 36, 26);
  updateStatus(true, "starting");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if (!connect_tello()) {
    return;
    //error();
  }
  strcpy(Tello.address, TelloAddress);
  Tello.commandPortNo = TelloCommandPort;
  updateStatus(true, NULL);
  send("command");
}

void execute()
{
  switch (Tello.mode) {
  case tm_takeoff:
    send("takeoff");
    break;
  case tm_land:
    send("land");
    break;
  case tm_reset:
    ESP.restart();
    break;
  }
}

void loop()
{
  M5.update();
  if (M5.BtnA.wasPressed()) {
    execute();
    delay(100);
  }
  if (M5.BtnB.wasPressed()) {
    Tello.next();
    updateStatus(true, NULL);
    delay(100);
  }
  if (Serial2.available()) {
    send(Serial2.readString().c_str());
  }
}
