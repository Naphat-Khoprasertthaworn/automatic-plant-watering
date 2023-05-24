
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

EspSoftwareSerial::UART testSerial;

const char* ssid = "ssid";
const char* password = "password";

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "mqtt_Client";
const char* mqtt_username = "mqtt_username";
const char* mqtt_password = "mqtt_password";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[100];

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.setCallback(callback);
      client.subscribe("@msg/water"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  String tpc;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);

  if (String(topic)=="@msg/water"){
    if (message == "on"){
      Serial.print("work on");
      sendToSTM(1);
    }else{
      Serial.print("work off");
      sendToSTM(0);
    }
  }
}

void setup() {
  testSerial.begin(9600, EspSoftwareSerial::SWSERIAL_8N1, D7, D8, false, 128, 11);
  Serial.begin(9600);
  wifiConnect();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void wifiConnect() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

uint8_t c = 0;
void loop() {
  if (c%1000!=0){
    c+=1;
  }else{
    reconnect();
  }
  if (!client.connected()) {
    c+=1;
  }

  char *buffer[3];
  char data[100];
  
  while (testSerial.available() > 4) {
    String str = testSerial.readStringUntil('\n');
    
    const char delimiter[] = " ";
    char *token = strtok(const_cast<char*>(str.c_str()), delimiter);
    int i = 0;
    while (token != NULL) {
        Serial.println(token);
        if ( ((String)token).length() > 2){
          return;
        }
        buffer[i] = token;
        token = strtok(NULL, delimiter);
        i++;
    }
    Serial.println(str);
    sprintf( data,"{\"data\":{\"brightness\":%s ,\"moisture\":%s,\"temperature\":%s}}",buffer[0],buffer[1],buffer[2] );
    Serial.println(data);
    client.publish("@shadow/data/update", data);
    yield();
  }

  if (!client.connected()) {
    return;
  }
  client.loop();
  
}

void sendToSTM(int sig){
  char data[1];
  sprintf(data,"%d",sig);
  testSerial.write(data,1);
}