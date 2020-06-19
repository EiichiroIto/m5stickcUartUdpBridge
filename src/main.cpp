#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define TelloAddress "192.168.10.1"
#define TelloCommandPort 8889

class TelloInfo {
  public:
    char ssid[128];
    char address[16];
    int commandPortNo;
    char status[80];
    bool connected;
    char message[256];

    TelloInfo() {
      ssid[0] = '\0';
      address[0] = '\0';
      status[0] = '\0';
      message[0] = '\0';
      connected = false;
    };
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
  if (strlen(buf) > sizeof(Tello.message) - 1) {
    updateStatus(true, "too long message");
    return;
  }
  Udp.beginPacket(Tello.address, Tello.commandPortNo);
  Udp.write((const uint8_t *) buf, strlen(buf));
  Udp.endPacket();  
  strcpy(Tello.message, buf);
  updateStatus(false, "sent");
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

void loop()
{
  M5.update();
  if (M5.BtnA.wasPressed()) {
    send("takeoff");
    delay(100);
  }
  if (M5.BtnB.wasPressed()) {
    send("land");
    delay(100);
  }
  if (Serial2.available()) {
    send(Serial2.readString().c_str());
  }
}
