#include <OneWire.h>
#include <SPI.h>           // Ethernet shield
#include <Ethernet.h>      // Ethernet shield
#include <PubSubClient.h>  // MQTT

OneWire  ds(5);
#define GPIO2_pin 2
#define GPIO3_pin 3
int sensePin = 0;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xE4, 0xDE, 0x35 }; // MAC-адрес нашего устройства
byte ip[] = { 192, 168, 1, 120 };
byte subnet[] = { 255, 255, 255, 0 };
byte gateway[] = { 192, 168, 1, 1 };
byte dns_server[] = { 192, 168, 1, 1 };

unsigned long lastMqtt = 0;
char mqtt_server[] = "m21.cloudmqtt.com";
char mqtt_user[] = "wvzaejdb";
char mqtt_pass[] = "-rewPZuV-ilM";
char mqtt_client[] = "Ethernet";

EthernetClient ethClient;
PubSubClient client(mqtt_server, 12787, ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strPayload);

  if (strTopic == "/smart-home/in/GPIO2") {
    if (strPayload == "OFF")     digitalWrite(GPIO2_pin, LOW);
    else if (strPayload == "ON") digitalWrite(GPIO2_pin, HIGH);
  }
  else if (strTopic == "/smart-home/in/GPIO3") {
    if (strPayload == "OFF") digitalWrite(GPIO3_pin, LOW);
    else if (strPayload == "ON") digitalWrite(GPIO3_pin, HIGH);
  }
 
  else  if (strTopic == "/smart-home/in/device-info") {
    if (strPayload == "GPIO2")  client.publish("/smart-home/out/GPIO2", "{\n \"type\": \"relay\", \n \"metrics\": \"ON/OFF\" \n }");
    else if (strPayload == "GPIO3") client.publish("/smart-home/out/GPIO3", "{\n \"type\": \"relay\", \n \"metrics\": \"ON/OFF\" \n }");
    else if (strPayload == "light")  client.publish("/smart-home/out/light", "{\n \"type\": \"sensor\", \n \"metrics\": \"int\" \n }");
    else if (strPayload == "temperature")  client.publish("/smart-home/out/temperature", "{\n \"type\": \"sensor\", \n \"double\": \"ON/OFF\" \n }");
    else if (strPayload == "devices") {
      client.publish("/smart-home/out/GPIO2", "{\n \"type\": \"relay\", \n \"metrics\": \"ON/OFF\" \n }");
      client.publish("/smart-home/out/GPIO3", "{\n \"type\": \"relay\", \n \"metrics\": \"ON/OFF\" \n }");
      client.publish("/smart-home/out/light", "{\n \"type\": \"sensor\", \n \"metrics\": \"int\" \n }");
      client.publish("/smart-home/out/temperature", "{\n \"type\": \"sensor\", \n \"double\": \"ON/OFF\" \n }");
    }
  }
}


void setup() {
  Serial.begin(57600);
  Serial.println("start");

  pinMode(GPIO2_pin, OUTPUT);
  digitalWrite(GPIO2_pin, LOW);
  pinMode(GPIO3_pin, OUTPUT);
  digitalWrite(GPIO3_pin, LOW);

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");


  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker ");
    if (client.connect(mqtt_client, mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT broker");
      client.setCallback(callback);
      //  client.publish("/smart-home/out/GPIO2", "OFF");
      // client.publish("/smart-home/out/GPIO3", "OFF");
      client.subscribe("/smart-home/in/#");
    } else {
      Serial.println("Connecting to MQTT broker failed");
      Serial.println(client.state());
      delay(1000);
    }
  }
}

void loop() {
  if (lastMqtt > millis()) lastMqtt = 0;
  client.loop();
  // здесь какой-то другой код по уравлению светом, например, с кнопок или ещё как

  if (millis() > (lastMqtt + 10000)) {
    if (!client.connected()) {
      if (client.connect("smart-home")) client.subscribe("/smart-home/in/#");
    }
  }

  Serial.println(analogRead(sensePin));
  // String lightVal = String(analogRead(sensePin));
  char lightValChar[4];
  dtostrf(analogRead(sensePin), 4, 0, lightValChar);
  // Read current temperature and
  // publish it
  float celsius;
  char temp[10];
  celsius = readTemperature();
  dtostrf(celsius, 2, 2, temp);

  if (client.connected()) {
    if (celsius && celsius != -300.00) {
      client.publish("/smart-home/out/temperature", temp);
    }
    client.publish("/smart-home/out/light", lightValChar);
    if (digitalRead(GPIO2_pin)) client.publish("/smart-home/out/GPIO2", "ON");
    else client.publish("/smart-home/out/GPIO2", "OFF");
    if (digitalRead(GPIO3_pin)) client.publish("/smart-home/out/GPIO3", "ON");
    else client.publish("/smart-home/out/GPIO3", "OFF");
  }
  lastMqtt = millis();
}

// Temperature reading from DS18B20 module
// Code is from OneWire library example
float readTemperature() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  // Can I haz an address?
  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return -300.00;
  }
  // Is CRC valid?
  if (OneWire::crc8(addr, 7) != addr[7]) {
    return -200.00;
  }
  ds.reset();
  ds.select(addr);
  // Start conversion, with parasite power on at the end
  ds.write(0x44, 1);
  delay(1000);
  // We might do a ds.depower() here, but the reset will take care of it.
  present = ds.reset();
  ds.select(addr);
  // Read Scratchpad
  ds.write(0xBE);
  // Read 9 bytes
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  return celsius;
}


