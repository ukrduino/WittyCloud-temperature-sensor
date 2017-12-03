#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>

// Update these with values suitable for your network.
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

// Green led
const int LED = 12;

//Temp sensor
#define ONE_WIRE_BUS 14                            // Digital pin DS18B20 is connected to.
#define TEMPERATURE_PRECISION 12                   // Set sensor precision.  Valid options 8,10,11,12 Lower is faster but less precise

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// arrays to hold device address
DeviceAddress insideThermometer;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];


void setup() {
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  processTempSensor();
}

void processTempSensor() {
  sensors.begin();
  if (!sensors.getAddress(insideThermometer, 0)) {
    Serial.println("Unable to find address for Device 0");
  }

  Serial.print("Setting resolution to ");
  Serial.println(TEMPERATURE_PRECISION, DEC);

  // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);

  Serial.print("Resolution actually set to: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
}


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

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED, HIGH);   // Turn the LED on
  }
  if ((char)payload[0] == '0') {
    digitalWrite(LED, LOW);  // Turn the LED off
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("WittyCloud/temp", "ESP8266Client connected");
      // ... and resubscribe
      client.subscribe("WittyCloud/input");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  sendTempToMqtt();
}

void sendTempToMqtt() {
  long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
//    Serial.print("Publish message: ");
//    Serial.println(temp);
    client.publish("WittyCloud/temp", String(temp).c_str());
  }
}

