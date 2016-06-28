#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>  // MQTT
#include <ESP8266WiFi.h>
#include <Servo.h>

#define RELAY_1 13
#define RELAY_2 5
#define RELAY_3 4
#define DHTPIN_inside 14
#define DHTPIN_outside 2
#define servo_pin 12


const char *ssid =  "GagarinM";
const char *pass =  "95628767";


unsigned long lastMqtt = 0;
char mqtt_server[] = "m21.cloudmqtt.com";
char mqtt_user[] = "wvzaejdb";
char mqtt_pass[] = "-rewPZuV-ilM";
char mqtt_client[] = "WiFi";

DHT dht_inside(DHTPIN_inside, DHT22);
DHT dht_outside(DHTPIN_outside, DHT11);
Servo myservo;
WiFiClient WifiClient;
PubSubClient client(mqtt_server, 12787, WifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strPayload);

  if (strTopic == "/smart-home/in/RELAY_1") {
    if (strPayload == "OFF")     digitalWrite(RELAY_1, LOW);
    else if (strPayload == "ON") digitalWrite(RELAY_1, HIGH);
  }
  else if (strTopic == "/smart-home/in/RELAY_2") {
    if (strPayload == "OFF") digitalWrite(RELAY_2, LOW);
    else if (strPayload == "ON") digitalWrite(RELAY_2, HIGH);
  }
  else if (strTopic == "/smart-home/in/RELAY_3") {
    if (strPayload == "OFF") digitalWrite(RELAY_3, LOW);
    else if (strPayload == "ON") digitalWrite(RELAY_3, HIGH);
  }
  else if (strTopic == "/smart-home/in/servo") {
    myservo.write(strPayload.toInt());
  }
  else if (strTopic == "/smart-home/in/device-info") {
    if (strPayload == "temperature_outside")  {
      client.publish("/smart-home/out/temperature_outside", "{\n \"type\": \"sensor\", \n \"metrics\": \"double\" \n }");
      Serial.println("{\n \"type\": \"sensor\", \n \"metrics\": \"double\" \n }");
      delay(500);
    }
    else if (strPayload == "temperature_inside")  {
      client.publish("/smart-home/out/temperature_inside", "{\n \"type\": \"sensor\", \n \"metrics\": \"double\" \n }");
      Serial.println("{\n \"type\": \"sensor\", \n \"metrics\": \"double\" \n }");
      delay(500);
    }
    else if (strPayload == "humidity_outside")  {
      client.publish("/smart-home/out/humidity_outside", "{\n \"type\": \"sensor\", \n \"metrics\": \"int\" \n }");
      Serial.println("{\n \"type\": \"sensor\", \n \"metrics\": \"int\" \n }");
      delay(500);
    }
    else if (strPayload == "humidity_inside")  {
      client.publish("/smart-home/out/humidity_inside", "{\n \"type\": \"sensor\", \n \"metrics\": \"int\" \n }");
      Serial.println("{\n \"type\": \"sensor\", \n \"metrics\": \"int\" \n }");
      delay(500);
    }
    else if (strPayload == "servo")  {
      client.publish("/smart-home/out/servo", "{\n \"type\": \"servo\", \n \"metrics\": \"int\" \n }");
      Serial.println("{\n \"type\": \"servo\", \n \"metrics\": \"int\" \n }");
      delay(500);
    }
    else if (strPayload == "RELAY_1")  {
      client.publish("/smart-home/out/RELAY_1", "{\n \"type\": \"relay\", \n \"metrics\": \"switcher\" \n }");
      Serial.println("{\n \"type\": \"relay\", \n \"metrics\": \"switcher\" \n }");
      delay(500);
    }
    else if (strPayload == "RELAY_2") {
      client.publish("/smart-home/out/RELAY_2", "{\n \"type\": \"relay\", \n \"metrics\": \"switcher\" \n }");
      Serial.println("{\n \"type\": \"relay\", \n \"metrics\": \"switcher\" \n }");
      delay(500);
    }
    else if (strPayload == "RELAY_3")  {
      client.publish("/smart-home/out/RELAY_3", "{\n \"type\": \"relay\", \n \"metrics\": \"switcher\" \n }");
      Serial.println("{\n \"type\": \"relay\", \n \"metrics\": \"switcher\" \n }");
      delay(500);
    }
  }
}

void restart() {
  Serial.println("Will reset and try again...");
  abort();
}

void setup() {
  Serial.begin(9600);
  Serial.println("start");
  dht_inside.begin();
  dht_outside.begin();
  myservo.attach(servo_pin);

  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, LOW);
  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, LOW);
  pinMode(RELAY_3, OUTPUT);
  digitalWrite(RELAY_3, LOW);

  WiFi.begin(ssid, pass);

  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 10)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.printDiag(Serial);
  } else {
    restart();
  }

  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker ");
    if (client.connect(mqtt_client, mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT broker");
      client.setCallback(callback);
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

  if (client.connected()) {

    float t_outside = dht_outside.readTemperature();
    char outsideTemp[5];
    dtostrf(t_outside, 2, 2, outsideTemp);
    client.publish("/smart-home/out/temperature_outside", outsideTemp);
    Serial.println("Outside temp ");
    Serial.print(outsideTemp);
    Serial.println();

    float t_inside = dht_inside.readTemperature();
    char insideTemp[5];
    dtostrf(t_inside, 2, 2, insideTemp);
    client.publish("/smart-home/out/temperature_inside", insideTemp);
    Serial.println("Inside temp ");
    Serial.print(insideTemp);
    Serial.println();

    float h_outside = dht_outside.readHumidity();
    char outsideHum[3];
    dtostrf(h_outside, 3, 0, outsideHum);
    client.publish("/smart-home/out/humidity_outside", outsideHum);
    Serial.println("Outside hum ");
    Serial.print(outsideHum);
    Serial.println();

    float h_inside = dht_inside.readHumidity();
    char insideHum[3];
    dtostrf(h_inside, 3, 0, insideHum);
    client.publish("/smart-home/out/humidity_inside", insideHum);
    Serial.println("Inside hum ");
    Serial.print(insideHum);
    Serial.println();

    int servo_read = myservo.read();
    char servo_pos[3];
    dtostrf(servo_read, 3, 0, servo_pos);
    client.publish("/smart-home/out/servo", servo_pos);

    if (digitalRead(RELAY_1)) client.publish("/smart-home/out/RELAY_1", "ON");
    else client.publish("/smart-home/out/RELAY_1", "OFF");
    if (digitalRead(RELAY_2)) client.publish("/smart-home/out/RELAY_2", "ON");
    else client.publish("/smart-home/out/RELAY_2", "OFF");
    if (digitalRead(RELAY_3)) client.publish("/smart-home/out/RELAY_3", "ON");
    else client.publish("/smart-home/out/RELAY_3", "OFF");
  }
  lastMqtt = millis();
}

