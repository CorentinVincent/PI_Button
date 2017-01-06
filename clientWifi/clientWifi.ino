
#include <ESP8266WiFi.h>

const char* ssid = "SFR_C1C8";
const char* password = "rsmoeumebrof7ortampl";

char* host = "httpbin.org";

const int buttonPin = 12;  
const int ledPinJaune =  4;
const int ledPinVerte = 13;
const int ledPinRouge =  5;

 
static int buttonState = 0;


void setup() {
  Serial.begin(115200);
  delay(10);

  //Allumage de la led rouge pour indiquer que l'esp8266 n'est pas connecté au WiFi
  digitalWrite(ledPinRouge, HIGH);
  delay(2000);
  digitalWrite(ledPinRouge, LOW);
  
   //Paramétrage des PINs utilisés (entrée ou sortie)
  pinMode(ledPinJaune, OUTPUT);
  pinMode(ledPinVerte, OUTPUT);
  pinMode(ledPinRouge, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  //Tentative de connection au WiFi avec le ssid et password
  Serial.println();
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

  //Allumage de la led verte pour indiquer que l'esp8266 est connecté au WiFi
  digitalWrite(ledPinVerte, HIGH);
  delay(2000);
  digitalWrite(ledPinVerte, LOW);
}

void loop() {

  buttonState = digitalRead(buttonPin);
  
  if (buttonState == HIGH) {

    //Allumage de la led verte pour indiquer que l'on fait une requete
    digitalWrite(ledPinVerte, HIGH);
    
    Serial.print("connecting to ");
    Serial.println(host);
    
    // Utilisation de la classe WiFiClient pour créer une connexion TCP
    WiFiClient client;
    //Vérification de l'état de la connexion au serveur 
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    
    //Creation d'une URL pour la requête
    String url = "/get";
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    //Envoie de la requête au serveur
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    
    // Lecture de toutes les lignes retourner par le serveur et impression de ces dernière dans le port série
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
    Serial.println();
    Serial.println("closing connection");
    
    //Extinction de la led verte
    digitalWrite(ledPinVerte, LOW);
  }
  else{
    Serial.println("Pas de requete a envoye");
    digitalWrite(ledPinRouge, HIGH);
    delay(2000);
    digitalWrite(ledPinRouge, LOW);
  }
}
