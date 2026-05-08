#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Audio.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

// ================= CONFIG =================
Preferences prefs;
WebServer server(80);
Audio audio;

// ================= OLED =================
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ================= BELL ICON =================
const unsigned char bell_icon [] PROGMEM = {
  0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0,
  0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0,
  0x1f, 0xf8, 0x1f, 0xf8, 0x00, 0x00, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00
};

// ================= VARIABLES =================
String ssid, pass;
String radioURL;
String ntpServer;
String tz = "ART3";

String alarms[5];
String alarmDays[5];

// ================= ALARM CONFIG =================
bool alarmFade = true;
int alarmMaxVolume = 18;
int fadeVolume = 1;

// ⏱️ AUTO OFF ALARMA
int alarmAutoOffMin = 30;
unsigned long alarmStartMillis = 0;

// 🚫 DESACTIVAR TODAS LAS ALARMAS
bool alarmsDisabled = false;

// ================= STATUS =================
bool isPlaying = false;
bool alarmActive = false;
bool alarmFiredToday[5] = {false,false,false,false,false};

// ================= TIME =================
struct tm timeinfo;

// ================= BUTTON =================
#define BTN 14

// ================= AUDIO TASK =================
void audioTask(void *param) {
  while (true) {
    audio.loop();
    vTaskDelay(1);
  }
}

// ================= iRADIO =================
void startRadio(int vol = -1) {

  audio.stopSong();
  delay(200);

  isPlaying = false;

  audio.connecttohost(radioURL.c_str());
  isPlaying = true;

  if (vol >= 0) audio.setVolume(vol);
}

void stopRadio() {
  audio.stopSong();
  isPlaying = false;
}

// ================= UTIL =================
int toMinutes(String t) {
  return t.substring(0,2).toInt() * 60 +
         t.substring(3,5).toInt();
}

// ================= ALARMS =================
void loadAlarms() {
  prefs.begin("cfg", true);

  for (int i = 0; i < 5; i++) {
    alarms[i] = prefs.getString(("a" + String(i)).c_str(), "07:00");
    alarmDays[i] = prefs.getString(("d" + String(i)).c_str(), "1111111");
  }

  alarmFade = prefs.getBool("fade", true);
  alarmMaxVolume = prefs.getInt("amax", 18);
  alarmAutoOffMin = prefs.getInt("aoff", 30);
  alarmsDisabled = prefs.getBool("adisable", false);

  prefs.end();
}

void saveAlarms() {
  prefs.begin("cfg", false);

  for (int i = 0; i < 5; i++) {
    prefs.putString(("a" + String(i)).c_str(), alarms[i]);
    prefs.putString(("d" + String(i)).c_str(), alarmDays[i]);
  }

  prefs.putBool("fade", alarmFade);
  prefs.putInt("amax", alarmMaxVolume);
  prefs.putInt("aoff", alarmAutoOffMin);
  prefs.putBool("adisable", alarmsDisabled);

  prefs.end();
}

// ================= AP =================
void startAP() {
  WiFi.softAP("Esp32 Clock Radio", "ESP32Clock");
}

// ================= WEB =================
void handleRoot() {

  String html =
  "<!DOCTYPE html><html><head>"
  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
  "<style>"
  "body{font-family:Arial;background:#111;color:#eee;margin:0;padding:20px;}"
  ".card{background:#1c1c1c;padding:15px;border-radius:12px;margin-bottom:15px;}"
  "h2{margin:0 0 10px 0;font-size:18px;color:#4fc3f7;}"
  "input{padding:6px;margin:3px;border-radius:6px;border:none;}"
  "button{width:100%;padding:10px;background:#4fc3f7;border:none;border-radius:8px;color:#000;font-weight:bold;}"
  "</style></head><body>";

  html += "<form method='POST' action='/save'>";

  html += "<h2>ESP32 Clock Radio</h2>";

  html += "<div class='card'><h2>WiFi</h2>";
  html += "SSID<input name='s' value='" + ssid + "'>";
  html += "PASS<input name='p' value='" + pass + "'>";
  html += "</div>";

  html += "<div class='card'><h2>Radio</h2>";
  html += "URL<input name='r' value='" + radioURL + "'>";
  html += "<br>NTP<input name='n' value='" + ntpServer + "'>";
  html += "</div>";

  html += "<div class='card'><h2>Alarmas</h2>";

  html += "Desactivar todas ";
  html += "<input type='checkbox' name='disableAlarms' ";
  if (alarmsDisabled) html += "checked";
  html += "><br><br>";

  const char* dnames[7] = {"L","M","X","J","V","S","D"};

  for (int i = 0; i < 5; i++) {

    html += "Alarma " + String(i+1);
    html += "<input name='a" + String(i) + "' value='" + alarms[i] + "'>";

    for (int d = 0; d < 7; d++) {
      html += dnames[d];
      html += "<input type='checkbox' name='d" + String(i) + "_" + String(d) + "' ";
      if (alarmDays[i].length() == 7 && alarmDays[i][d] == '1') html += "checked";
      html += ">";
    }
    html += "<br>";
  }

  html += "</div>";

  html += "<div class='card'><h2>Configuracion alarma</h2>";

  html += "Fade <input type='checkbox' name='fade' ";
  if (alarmFade) html += "checked";
  html += "><br>";

  html += "Volumen<input type='range' min='5' max='21' name='amax' value='" +
          String(alarmMaxVolume) + "'>";

  html += "<br>Silenciar alarmas despues de <input type='number' name='aoff' value='" +
          String(alarmAutoOffMin) + "'>";

  html += "</div>";

  html += "<button type='submit'>Guardar</button>";
  html += "</form></body></html>";

  server.send(200, "text/html", html);
}

// ================= SAVE CONFIG =================
void handleSave() {
  prefs.begin("cfg", false);

  ssid = server.arg("s");
  pass = server.arg("p");
  radioURL = server.arg("r");
  ntpServer = server.arg("n");

  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.putString("radio", radioURL);
  prefs.putString("ntp", ntpServer);

  for (int i = 0; i < 5; i++) {

    alarms[i] = server.arg("a" + String(i));

    String dstr = "";
    for (int d = 0; d < 7; d++) {
      if (server.hasArg("d" + String(i) + "_" + String(d))) dstr += "1";
      else dstr += "0";
    }
    alarmDays[i] = dstr;
  }

  alarmFade = server.hasArg("fade");
  alarmMaxVolume = server.arg("amax").toInt();
  alarmAutoOffMin = server.arg("aoff").toInt();
  alarmsDisabled = server.hasArg("disableAlarms");

  saveAlarms();

  prefs.end();

  server.send(200, "text/html", "Guardado. Reiniciando...");
  delay(1000);
  ESP.restart();
}

// ================= CLOCK =================
void drawClock() {
  if (!getLocalTime(&timeinfo)) return;

  display.clearDisplay();

  char hora[10];
  strftime(hora, sizeof(hora), "%H:%M", &timeinfo);

  display.setTextSize(4);
  display.setCursor(0, 0);
  display.println(hora);

  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println(WiFi.localIP().toString());

  if (!alarmsDisabled) {

    bool hasAlarm = false;
    for (int i = 0; i < 5; i++) {
      if (alarms[i].length() >= 4) {
        hasAlarm = true;
        break;
      }
    }

    static float vx = 0, vy = 0;
    static bool bellShake = false;
    static unsigned long bellShakeStart = 0;

    if (hasAlarm) {

      if (alarmActive && !bellShake) {
        bellShake = true;
        bellShakeStart = millis();
      }

      if (millis() - bellShakeStart > 2500) {
        bellShake = false;
      }

      if (bellShake) {
        vx += random(-2, 3);
        vy += random(-2, 3);

        vx *= 0.75;
        vy *= 0.75;

        if (abs(vx) < 0.3 && abs(vy) < 0.3) {
          vx = 0;
          vy = 0;
          bellShake = false;
        }
      }

      int offsetX = (int)vx;
      int offsetY = (int)vy;

      display.drawBitmap(128 - 25 + offsetX,
                         45 + offsetY,
                         bell_icon, 16, 16, WHITE);
    }
  }

  display.display();
}

// ================= ALARMS =================
void checkAlarms() {

  if (alarmsDisabled) return;
  if (!getLocalTime(&timeinfo)) return;

  int nowMin = timeinfo.tm_hour * 60 + timeinfo.tm_min;

  int today = timeinfo.tm_wday;
  int dayIndex = (today == 0) ? 6 : today - 1;

  for (int i = 0; i < 5; i++) {

    int alarmMin = toMinutes(alarms[i]);

    bool dayEnabled = false;
    if (alarmDays[i].length() == 7) {
      dayEnabled = alarmDays[i][dayIndex] == '1';
    }

    if (dayEnabled &&
        nowMin >= alarmMin &&
        nowMin < alarmMin + 1 &&
        !alarmFiredToday[i]) {

      alarmFiredToday[i] = true;
      alarmActive = true;
      alarmStartMillis = millis();

      fadeVolume = 1;
      audio.setVolume(1);

      startRadio(1);
    }

    if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0) {
      alarmFiredToday[i] = false;
    }
  }
}

// ================= AUTO OFF =================
void checkAlarmTimeout() {
  if (!alarmActive || !isPlaying) return;

  unsigned long elapsedMin = (millis() - alarmStartMillis) / 60000;

  if (elapsedMin >= alarmAutoOffMin) {
    stopRadio();
    alarmActive = false;
  }
}

// ================= ALARM FADE =================
void updateAlarmFade() {
  if (!alarmActive || !isPlaying) return;

  if (!alarmFade) {
    audio.setVolume(alarmMaxVolume);
    return;
  }

  static int lastVol = -1;

  if (lastVol == -1) lastVol = 1;

  if (lastVol < alarmMaxVolume) {
    lastVol++;
    audio.setVolume(lastVol);
    delay(1200);
  }
}

// ================= WIFI =================
void connectWiFi() {
  WiFi.setSleep(false);

  prefs.begin("cfg", true);

  ssid = prefs.getString("ssid", "");
  pass = prefs.getString("pass", "");
  radioURL = prefs.getString("radio",
    "http://stream-uk1.radioparadise.com/mp3-128");
  ntpServer = prefs.getString("ntp", "pool.ntp.org");

  loadAlarms();

  prefs.end();

  if (ssid == "") {
    startAP();
    return;
  }

  WiFi.begin(ssid.c_str(), pass.c_str());

  int t = 0;
  while (WiFi.status() != WL_CONNECTED && t < 20) {
    delay(500);
    t++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    startAP();
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(BTN, INPUT_PULLUP);

  Wire.begin(18, 19);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);

  connectWiFi();

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();

  configTzTime(tz.c_str(), ntpServer.c_str());

  audio.setBufsize(1024*8, 1024*8);
  audio.setPinout(26, 25, 27);
  audio.setVolume(10);

  xTaskCreatePinnedToCore(audioTask, "Audio", 10000, NULL, 2, NULL, 1);
}

// ================= LOOP =================
void loop() {
  server.handleClient();

  drawClock();
  checkAlarms();
  updateAlarmFade();
  checkAlarmTimeout();

  if (digitalRead(BTN) == LOW) {
    delay(200);

    if (isPlaying) {
      stopRadio();
      alarmActive = false;
    } else {
      startRadio(alarmMaxVolume);
    }

    while (digitalRead(BTN) == LOW);
  }

  delay(200);
}
