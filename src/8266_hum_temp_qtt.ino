/*
  Projet d'apprentissage d'un objet connecté (IoT)  pour réaliser une sonde de température
  ESP8266 + DHT22 + LED + MQTT + Home-Assistant
  Projets DIY (http://www.projetsdiy.fr) - Mai 2016
  Article du projet : http://www.projetsdiy.fr/esp8266-dht22-mqtt-projet-objet-connecte/
  Licence : MIT
*/

/*
 *Smart Farming 
 *Version: 0.1
 *License: MIT
 *Soenke Brix (http://www.brix-net.de)
 *The code is based on the stuff written by the guy above, thx mate!
 */
 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"          // DHT library

#define wifi_ssid "xxx"       //wifi network
#define wifi_password "xxx"   // passphrase

#define mqtt_server "192.168.178.61"  //ip MQTT broker
#define mqtt_user "pi"  //mosquitto user
#define mqtt_password "xxx" //passphrase

#define temperature_topic "sensor/temperature"  //Topic temperature
#define humidity_topic "sensor/humidity"        //Topic humidity

//Buffer mqtt message size
char message_buff[100];

long lastMsg = 0;   //counter MQTT message
long lastRecu = 0;
bool debug = true;  //if TRUE debug messages in console

#define DHTPIN D4    // DHT pin

// uncomment used DHT type
//#define DHTTYPE DHT11       // DHT 11 
#define DHTTYPE DHT22         // DHT 22  (AM2302)

//create objects
DHT dht(DHTPIN, DHTTYPE);     
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);     //if debug is true
  pinMode(D2,OUTPUT);     //Pin 2 in output mode for LED
  setup_wifi();           //start wifi
  client.setServer(mqtt_server, 1883);    //initialize MQTT server
  client.setCallback(callback);  //???   
  dht.begin();  //start DHT
}

//Start wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WIFI Connection estabished ");
  Serial.print(" => Addresse IP : ");
  Serial.print(WiFi.localIP());
}
 
//reconnection
void reconnect() {
  //Loop until a reconnection is obtained
  while (!client.connected()) {
    Serial.print("Connecting to the MQTT server...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println(" We wait five seconds before we do it again.");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  //Sending one message per minute
  if (now - lastMsg > 1000 * 10) {
    lastMsg = now;
    //Ambient humidity reading
    float h = dht.readHumidity();
    // Temperature reading in Celcius
    float t = dht.readTemperature();

    //There's no point in going any further if the sensor doesn't return anything.
    /*
    if ( isnan(t) || isnan(h)) {
      Serial.println("Failed to read! Check your DHT sensor");
      return;
    }
    */
  
    if ( debug ) {
      Serial.print("Temperature : ");
      Serial.print(t);
      Serial.print(" | Humidite : ");
      Serial.println(h);
    }  
    client.publish(temperature_topic, String(t).c_str(), true);   //Publish the temperature on the topic temperature_topic
    client.publish(humidity_topic, String(h).c_str(), true);      //And the humidity
  }
  if (now - lastRecu > 100 ) {
    lastRecu = now;
    client.subscribe("homeassistant/switch1");
  }
}

// Triggers actions on receipt of a message
// According to http://m2mio.tumblr.com/post/30048662088/a-simple-example-arduino-mqtt-m2mio
void callback(char* topic, byte* payload, unsigned int length) {

  int i = 0;
  if ( debug ) {
    Serial.println("Message received =>  topic: " + String(topic));
    Serial.print(" | length: " + String(length,DEC));
  }
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  if ( debug ) {
    Serial.println("Payload: " + msgString);
  }
  
  if ( msgString == "ON" ) {
    digitalWrite(D2,HIGH);  
  } else {
    digitalWrite(D2,LOW);  
  }
}
