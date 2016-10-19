# How to...

1. Install Arduino IDE https://www.arduino.cc/en/Guide/HomePage and libraries for ESP8266 https://github.com/esp8266/Arduino
2. Add libraries: OneWire, ESP8266, ETHRENET, DHT, PubSubClient. Instructions https://www.arduino.cc/en/Guide/Libraries
3. In case you using ESP8266, open *WebServer_mqtt_ESP8266_full.ino* sketch, else you using arduino and eth shield, open *WebServer_mqtt_ds_devinfo.ino*
4. Edit sketch with your own data:
  for arduino:
    - char mqtt_server[] = ""; // server mqtt address
    - char mqtt_user[] = ""; // user name for mqtt server
    - char mqtt_pass[] = ""; // user password
  for ESP8266:
    - the same like for adrduino
    - const char *ssid =  ""; // your WiFi AP name
    - const char *pass =  ""; // WiFi password
5. Upload current sketch
