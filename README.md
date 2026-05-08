# esp32radio
ESP32 Radio Clock Firmware

This is a firmware for ESP32 to make a nice internet radio with alarm clock!

Be advised, this is a plain and simple firmware, you can modify as you wish to make it cooler!

# Components
- ESP32-WROOM-32
- SSD1306 display
- MAX98357A I2S Class D Amplifier
- Mini Speaker for Arduino
- Momentary switch push button

# Default GPIO
      SSD1306
      D18 SDA
      D19 SCL

      MAX98357A
      D26 BCLK
      D25 LRC
      D27 DIN

      BUTTON SWITCH
      D14

# What it does?
This firmware makes boot the ESP32 and creates an Access Point for you to connect the first time, common IP used on the ESP32 192.168.4.1, there you can configure your home's wifi.
Configured with your local wifi, it boots and shows the local IP on the SSD1306 display, access to it and you can configure:
- LOCAL WIFI
- RADIO URL (STREAMING ONE)
- NTP URL
- 5 Alarms and a deactivate box, to cancel all alarms
- Fade config, when your alarm activates the streaming start with low volume and increase over time
- Dead time config, set in minutes how much time to auto-deactivate a current alarm

Also, when no alarm is activated you can push the button and it start streaming your favorite radio, push it again to stop.

<img width="1899" height="776" alt="image" src="https://github.com/user-attachments/assets/1264a8e6-a4e5-4ac8-8797-992817b6dc41" />

# Code details

- If you want to config a different SSID and password for AP, you can change the next code line, line 124

      void startAP() {
        WiFi.softAP("Esp32 Clock Radio", "ESP32Clock");
      }

- There is a timezone hided because I did not use, line 29
Is not included in the HTML.

      String tz = "ART3";

- If you configure at least one alarm, a bell will display on the SSD1306, you can change this icon on line 19

       const unsigned char bell_icon [] PROGMEM = {
        0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0,
        0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0,
        0x1f, 0xf8, 0x1f, 0xf8, 0x00, 0x00, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00
      };

- When an alarm is activated, you will see the bell icon shaking on the display, you can change how much it moves on line 283

      if (bellShake) {
        vx += random(-2, 3);
        vy += random(-2, 3);

  # 3D PRINTING

  Yes, I had created a nice-looking box for this clock, you can check it at:

  https://www.thingiverse.com/thing:7348580
  
