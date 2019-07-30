#include <ESP8266WiFi.h>  // bibliothèque ! 
#include <PubSubClient.h> // bibliothèque ! 
#include <Adafruit_Sensor.h> // bibliothèque ! 
#include <DHT.h>

#define DHTTYPE DHT11   // DHT 11 

const char* ssid = "123france2018"; // Wifi login 
const char* password = "canadacanada123"; //Wifi password 
// MQTT broker
const char* mqtt_server = "broker.hivemq.com";  // votre server broker 
// Initialisation de espClient. Vous devriez changer le nom d'espClient si vous avez plusieurs ESPs en cours d'exécution  
WiFiClient espClient; 
PubSubClient client(espClient); 
// DHT Sensor - GPIO 5 = D1 on ESP-12E NodeMCU board 
const int DHTPin = 5; 
// Lamp - LED - GPIO 4 = D2 on ESP-12E NodeMCU board 
const int lamp = 4; 
// Smoke Sensor
const int smokePin = A0;
//GPIO 13 = D7 on ESP-12E NodeMCU board 
const int BleuLamp =13;
// Initialize DHT sensor. 
DHT dht(DHTPin, DHTTYPE);// Timers

// Smoke Threshold
int smokeThres = 60;

// Control Variables
boolean armSmoke = false;
boolean smokeTriggered = false;  
long now = millis(); 
long lastMeasure = 0; 
long lastSmokeCheck = 0;
// Ne modifiez pas la fonction ci-dessous. Cette fonction connecte votre ESP8266 à votre routeur. 
void setup_wifi() { 
  delay(10); 
  // Nous commençons par nous connecter à un réseau WiFi 
  Serial.println(); 
  Serial.print("Connecting to "); 
  Serial.println(ssid); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 
  Serial.println(""); 
  Serial.print("WiFi connected - ESP IP address: "); 
  Serial.println(WiFi.localIP()); 
} 
// Cette fonction est exécutée lorsqu'un appareil publie un message à un sujet auquel votre ESP8266 est abonné 
void callback(String topic, byte* message, unsigned int length) { 
  Serial.print("Message arrived on topic: "); 
  Serial.print(topic); 
  Serial.print(". Message: "); 
  String messageTemp; 
  for (int i = 0; i < length; i++) { 
    Serial.print((char)message[i]); 
    messageTemp += (char)message[i]; 
  } 
  Serial.println(); 
  // Si un message est reçu sur Topic : room/lamp, vous vérifiez si le message est allumé ou éteint 
  if(topic=="room/lamp"){ 
      Serial.print("Changing Room lamp to "); 
      if(messageTemp == "1"){ 
        digitalWrite(lamp, HIGH); 
        Serial.print("on"); 
      } 
      else if(messageTemp == "0"){ 
        digitalWrite(lamp, LOW); 
        Serial.print("Off"); 
      } 
  }
     /* if(topic=="room/gaz1"){
      Serial.print("SMOKE SENSOR STATUS CHANGE");
        if(messageTemp == "1"){
        Serial.print("Smoke Sensor Armed");     
        client.publish("room/gaz1", "NO SMOKE");
        armSmoke = true;
        smokeTriggered = false;
      }
      else if(messageTemp == "0"){
        Serial.print("Smoke Sensor Not Armed");      
        client.publish("room/gaz1", "NO SMOKE");
       
        smokeTriggered = false;
      }
  } */
  
  Serial.println(); 
  
} 
//************ Cette fonction reconnecte votre ESP8266 à votre courtier MQTT. ******************************************/
// Modifiez la fonction ci-dessous si vous souhaitez vous abonner à d'autres sujets avec votre ESP8266  
void reconnect() { 
  while (!client.connected()) { 
    Serial.print("Attempting MQTT connection..."); 
    if (client.connect("ESP8266Client")) { 
      Serial.println("connected");   
      // Subscribe or resubscribe to a topic 
      // You can subscribe to more topics (to control more LEDs in this example) 
      client.subscribe("room/lamp"); 
     client.subscribe("room/gaz1"); // MQ2
    } else { 
      Serial.print("failed, rc="); 
      Serial.print(client.state()); 
      Serial.println(" try again in 5 seconds"); 
      // Wait 5 seconds before retrying 
      delay(5000); 
    } 
  } 
} 
/* ******************************fin fonction reconnect()*******************************/
// démarre la communication série à une vitesse de transmission de 9600 bauds. 
// Définit votre courtier mqtt et définit la fonction callback 
// La fonction de rappel est ce qui reçoit les messages et contrôle réellement les LEDs. 
void setup() { 
  pinMode(lamp, OUTPUT); 
  pinMode(smokePin, INPUT);
  pinMode(BleuLamp, OUTPUT); 
  Serial.begin(9600); 
  setup_wifi(); 
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback); 
}
// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis(); 
  // publier la temperature et l’humidité chaque 30 seconde 
 if (now - lastMeasure > 30000) { 
    lastMeasure = now; 
    float h = dht.readHumidity(); 
    float t = dht.readTemperature(); 
    float f = dht.readTemperature(true); 
 
    if (isnan(h) || isnan(t) || isnan(f)) { 
      Serial.println("Failed to read from DHT sensor!"); 
      return; 
    } 
    float hic = dht.computeHeatIndex(t, h, false); 
    static char temperatureTemp[7]; 
    dtostrf(hic, 6, 2, temperatureTemp); 
    static char humidityTemp[7]; 
    dtostrf(h, 6, 2, humidityTemp); 
    client.publish("room/temperature1", temperatureTemp); 
    client.publish("room/humidity1", humidityTemp); 
     
    Serial.print("Humidity: "); 
    Serial.print(h); 
    Serial.print(" %\t Temperature: "); 
    Serial.print(t); 
    Serial.print(" *C "); 
    Serial.print(f); 
    Serial.print(" *F\t Heat index: "); 
    Serial.print(hic); 
    Serial.println(" *C "); 
   // Serial.print(hif); 
   // Serial.println(" *F"); 
  } 
   int smokeValue = analogRead(smokePin);
   // Checks smoke
   
  if (smokeValue> 530) {
      
      Serial.print("Pin A0: ");
      Serial.println(smokeValue);
        digitalWrite(BleuLamp, HIGH);

        Serial.println("GAZ DETECTED!!!");
        smokeTriggered = true;
        client.publish("room/gaz1", "GAZ DETECTED");
      
   
  }else {
    
        digitalWrite(BleuLamp, LOW);
    }

  
} 
