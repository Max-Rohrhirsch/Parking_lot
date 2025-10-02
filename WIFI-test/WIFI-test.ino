#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>   // OTA Updates für ESP32

// WLAN Daten
const char* ssid = "Xamklab";
const char* password = "studentXAMK";

// MQTT Broker Daten
const char* mqtt_broker = "172.20.53.121";  // IP ohne http://
const int   mqtt_port   = 1883;
const char* mqtt_user   = "student";
const char* mqtt_password = "student";

const char* HostName = "ESP32_Group3";  

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

// Globale Variablen
int iWiFiTry = 0;
int iMQTTTry = 0;
String sClient_id;

// WLAN Verbindung
void Connect2WiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setHostname(HostName);
  Serial.print("Connecting to WiFi ");

  iWiFiTry = 0;
  while (WiFi.status() != WL_CONNECTED && iWiFiTry < 20) {
    ++iWiFiTry;
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to WiFi");
  }

  // OTA Setup
  ArduinoOTA.setHostname(HostName);
  ArduinoOTA.onStart([]() { Serial.println("OTA Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("OTA End"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]\n", error);
  });
  ArduinoOTA.begin();

  // MQTT Client ID
  sClient_id = "ESP32-" + String(WiFi.macAddress());
  Serial.print("MQTT Client ID: ");
  Serial.println(sClient_id);
}

// MQTT Verbindung
void Connect2MQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    Connect2WiFi();
  }

  MQTTclient.setServer(mqtt_broker, mqtt_port);

  iMQTTTry = 0;
  while (!MQTTclient.connected() && iMQTTTry < 10) {
    Serial.print("Connecting to MQTT...");
    if (MQTTclient.connect(sClient_id.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
    iMQTTTry++;
  }
}

void setup() {
  Serial.begin(115200);
  Connect2WiFi();
  Connect2MQTT();

  // Test: publish
  MQTTclient.publish("parking/slots/available", "6");
}

void loop() {
  ArduinoOTA.handle();    // OTA am Laufen halten
  MQTTclient.loop();      // MQTT Verbindung aktiv halten
}
