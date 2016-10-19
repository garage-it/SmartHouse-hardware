#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>  // MQTT

#include <Ticker.h>

#define RELAY_1 12
#define LED 13
#define SWITCH 0

char ssid[] =  "GagarinM";
char pass[] =  "95628767";

unsigned long lastMqtt = 0;
char mqtt_server[] = "172.20.10.4";
char mqtt_user[] = "USERNAME";
char mqtt_pass[] = "PASSWORD";
char mqtt_client[] = "WiFi_itead_1";
int status = WL_IDLE_STATUS;

int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

Ticker ticker;

WiFiClient WifiClient;
PubSubClient client(mqtt_server, 1883, WifiClient);

void tick()
{
  //toggle state
  int state = digitalRead(LED);  // get the current state of LED pin
  digitalWrite(LED, !state);     // set pin to the opposite state
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strPayload);

  if (strTopic == "/smart-home/in/RELAY_1") {
    if (strPayload == "OFF") {
      digitalWrite(RELAY_1, LOW);
      state = LOW;
      digitalWrite(LED, HIGH);
      Serial.println("Relay off");
    }
    else if (strPayload == "ON") {
      digitalWrite(RELAY_1, HIGH);
      state = HIGH;
      digitalWrite(LED, LOW);
      Serial.println("Relay on");
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

  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, LOW);

  pinMode(LED, OUTPUT);
 // digitalWrite(LED, HIGH);

  pinMode(SWITCH, INPUT);

  ticker.attach(0.6, tick);

  WiFi.begin(ssid, pass);

  int retries = 0;
  Serial.println(WiFi.status());
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;
    delay(1000);
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
  ticker.attach(0.3, tick);
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
  ticker.detach();
}

void loop() {
  if (lastMqtt > millis()) lastMqtt = 0;
  client.loop();

    reading = digitalRead(SWITCH);
  
    if (reading == HIGH && previous == LOW) {
      if (state == HIGH) {
        state = LOW;
        delay(100);
      }
      else {
        state = HIGH;
        delay(100);
      }
    }
  
    digitalWrite(RELAY_1, state);
    digitalWrite(LED, !state);
    previous = reading;


  if (millis() > (lastMqtt + 10000)) {
    if (!client.connected()) {
      if (client.connect(mqtt_client)) client.subscribe("/smart-home/in/#");
    }
    if (client.connected()) {
      if (digitalRead(RELAY_1)) client.publish("/smart-home/out/RELAY_1", "ON");
      else client.publish("/smart-home/out/RELAY_1", "OFF");
    }
    lastMqtt = millis();
  }


}

