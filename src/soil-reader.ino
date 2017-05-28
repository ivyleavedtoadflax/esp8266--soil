#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

// Update these with values suitable for your network.

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_user = "";
const char* mqtt_password = "";

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// ADC stuff

Adafruit_ADS1015 ads1015;

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50]; // Message for publishing

int sensorPin = A0; // select the input pin for LDR
unsigned int sensorValue = 0; // variable to store the value coming from the sensor
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 60000;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  timeClient.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  ads1015.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  timeClient.update();

  unsigned long currentMillis = millis();

  int16_t adc0, adc1, adc2;

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      adc0 = ads1015.readADC_SingleEnded(0);
      adc1 = ads1015.readADC_SingleEnded(1);
      adc2 = ads1015.readADC_SingleEnded(2);
      //adc3 = ads1015.readADC_SingleEnded(3);

      // Get time from time server
      String formattedTime = timeClient.getFormattedTime();

      snprintf (msg, 75, " %ld", adc0);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("tele/esp/soil1", msg);

      snprintf (msg, 75, " %ld", adc1);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("tele/esp/soil2", msg);

      snprintf (msg, 75, " %ld", adc2);
      Serial.print(formattedTime);
      Serial.println(msg);
      client.publish("tele/esp/soil3", msg);

  }
}
