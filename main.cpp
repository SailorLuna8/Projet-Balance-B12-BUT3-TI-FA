
/* Programme permettant d'envoyer les données du capteur DHT11 à un broker via le protocole MQTT. */

// Bibliothèques utilisées

#include "Arduino.h" // Bibilothèque au language utilisé pour certaine fonction : Arduino 
#include "PubSubClient.h" // Bibliothèque associée au protocole MQTT
#include "WiFi.h" // Bibliothèque associée à l'accès au réseau utilisé : WiFi
#include "esp_wpa2.h" // Bibiothèque associée au protocole MQTT
#include "HX711.h" // Bibliothèque associée au capteur SEN-HX711-01

// Déclaration - Variable masse
float masse_lue; // Déclaration de la variable utilisée pour récupérer la masse lue par le capteur 

// Déclaration Capteur de force
const int LOADCELL_DOUT_PIN = 32; // Déclaration de la pin dont l'entrée correspond à la sortie de donnée du capteur sur la carte ESP32
const int LOADCELL_SCK_PIN = 33; // Déclaration de la pin dont l'entrée correspond à la sortie horloge du capteur sur la carte ESP32
HX711 scale; // Déclaration du capteur SEN-HX711-01


// Paramètres MQTT Broker

//const char *mqtt_broker = "broker.hivemq.com"; 
const char *mqtt_broker = "147.94.219.51"; // Identifiant du broker (Adresse IP)
const char *topic = "masse_lue"; // Nom du topic sur lequel les données seront envoyés, ici le nom du topic est "masse_lue"
const char *mqtt_username = ""; // Identifiant dans le cas d'une liaison sécurisée
const char *mqtt_password = ""; // Mdp dans le cas d'une liaison sécurisée
const int mqtt_port = 1883; // Port : 1883 dans le cas d'une liaison non sécurisée et 8883 dans le cas d'une liaison cryptée
WiFiClient espClient; 
PubSubClient client(espClient); 

// Paramètres EDUROAM
#define EAP_IDENTITY "selena.girard@etu.univ-amu.fr"
#define EAP_PASSWORD "Un1v3rs1t3-S3l3n4!" 
#define EAP_USERNAME "selena.girard@etu.univ-amu.fr" 
const char* ssid = "eduroam"; // eduroam SSID

// Fonction réception du message MQTT 
void callback(char *topic, byte *payload, unsigned int length) { 
  Serial.print("Le message a été envoyé sur le topic : "); 
  Serial.println(topic); 
  Serial.print("Message:"); 
  for (int i = 0; i < length; i++) { 
    Serial.print((char) payload[i]); 
  } 
  Serial.println(); 
  Serial.println("-----------------------"); 
}

// Initialisation du programme

void setup() { 
  Serial.begin(115200); // Déclaration du programme avec sa vitesse d'excécution = vitesse de calcul de la carte ESP32

// Initialisation du capteur HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); // Initialisation du capteur SEN-HX711-01
  scale.set_scale(1180.f);                      // Calibration du capteur avec un facteur de calibration = 1180 déterminé empiriquement avec des masses étalons
  scale.tare(); // Fonction qui effectue la tare du capteur 
 
// Connexion au réseau EDUROAM 

  WiFi.disconnect(true);
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println("");
  Serial.println(F("L'ESP32 est connecté au WiFi !"));
  
// Connexion au broker MQTT  
  
  client.setServer(mqtt_broker, mqtt_port); 
  client.setCallback(callback); 

  while (!client.connected()) { 
    String client_id = "esp32-client-"; 
    client_id += String(WiFi.macAddress()); 
    Serial.printf("La chaîne de mesure %s se connecte au broker MQTT", client_id.c_str()); 
 
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) { 
      Serial.println("La chaîne de mesure est connectée au broker."); 
    } else { 
      Serial.print("La chaîne de mesure n'a pas réussi à se connecter ... "); 
      Serial.print(client.state()); 
      delay(500); 
    } 
  } 
} 

void loop(){

  masse_lue = scale.get_units(5); // Fonction qui permet de stocker les valeurs lues par le capteur dans la variable "masse_lue"

  Serial.println (masse_lue); // Permet d'afficher la variable masse_lue sur le moniteur série de l'ordinateur 
  client.publish(topic, String(masse_lue).c_str()); // Publication de la variable masse_lue sur le topic (envoi d'une chaîne de caractères)
  client.subscribe(topic); // S'abonne au topic pour recevoir des messages
  client.loop(); // Gère les messages MQTT (pour lire la valeur de la température sur le moniteur série de platformIO)
  delay(500); // Pause de 5 secondes entre chaque envoi
}

