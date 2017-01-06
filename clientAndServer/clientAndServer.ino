/*
 * L'ESP8266 se connecte automatiquement au WiFi avec les paramètres passés
 * Suivant l'état du bouton poussoir :
 * Etat appuyé : l'ESP8266 agit comme un serveur WEB où l'on peut contrôller l'état des LEDs du montage
 * Etat relâché : l'ESP8266 agit comme un client qui requête le serveur passer en paramètre
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


//Variable globale pour se connecter au réseau local
const char* ssid = "BHC-Guest";
const char* password = "!Just4Guest!";

//Host à requeter quand l'ESP8266 est en mode client
char* host = "httpbin.org";

//Variable globale lorsque l'ESP8266 est en mode le serveur
MDNSResponder mdns;
ESP8266WebServer server(80);
String webPage = "";

//Declaration des PINs des GPIOs
const int buttonPin = 12;  
const int ledPinJaune =  4;
const int ledPinVerte = 13;
const int ledPinRouge =  5; 

static int buttonState = 0;

//Prototype des fonctions
String WifiScan();
String FlashConfig();
void serveur();

void setup() {

  // scanne du réseau Wi-Fi
  String res = WifiScan();
  //Ouverture à 115200 bauds du port série pour visualiser les printf
  Serial.begin(115200);
  delay(10);

   //Paramétrage des PINs utilisés (entrée ou sortie)
  pinMode(ledPinJaune, OUTPUT);
  pinMode(ledPinVerte, OUTPUT);
  pinMode(ledPinRouge, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  //Allumage de la led rouge pour indiquer que l'esp8266 n'est pas connecté au WiFi
  digitalWrite(ledPinRouge, HIGH);
  delay(2000);
  digitalWrite(ledPinRouge, LOW);
  
  Serial.println();
  Serial.println();
  Serial.print("Connexion a ");
  Serial.println(ssid);

  //Tentative de connection au WiFi avec le ssid et password
  WiFi.begin(ssid, password);

  //Attende de connection au WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  //Connection établie
  Serial.println("");
  Serial.println("WiFi connecte");  
  Serial.println("Addresse IP: ");
  Serial.println(WiFi.localIP());

  //Allumage de la led verte pour indiquer que l'esp8266 est connecté au WiFi
  digitalWrite(ledPinVerte, HIGH);
  delay(2000);
  digitalWrite(ledPinVerte, LOW);

  // Server
  
  //Contenu afficher sur la page web héberger par le serveur
  webPage +="<!DOCTYPE html><html><head><title>Serveur ESP8266</title></head><body>";
  webPage += "Adresse IP :";
  webPage += WiFi.localIP();
  webPage += "<h1>ESP8266 Web Server</h1><p>Led Verte <a href=\"socket1On\"><button>ON</button></a>&nbsp;<a href=\"socket1Off\"><button>OFF</button></a></p>";
  webPage += "<p>Led Rouge <a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"socket2Off\"><button>OFF</button></a></p>";
  webPage +="<h1>Configuration du Flash</h1>";
  webPage +="<p>";
  webPage += FlashConfig();
  webPage +="</p>";
  webPage +="<h1>Réseaux Wi-Fi</h1>";
  webPage += res;
  webPage += "</body></html>";

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder a demarre");
  }
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  server.on("/socket1On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(ledPinVerte, HIGH);
    delay(1000);
  });
  server.on("/socket1Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(ledPinVerte, LOW);
    delay(1000); 
  });
  server.on("/socket2On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(ledPinRouge, HIGH);
    delay(1000);
  });
  server.on("/socket2Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(ledPinRouge, LOW);
    delay(1000); 
  });
  server.begin();
  Serial.println("Le serveur HTTP a demarre");
 
}

int value = 0;

void loop() {

  //Variable pour différencier un appui long d'un court
  unsigned long time1, time2, timePushed;
  //Lecture de l'état du boutton poussoir
  buttonState = digitalRead(buttonPin);
  
  digitalWrite(ledPinRouge, HIGH);
 
  //Si boutton appuyé on lance le serveur
  if (buttonState == HIGH)
  {
    digitalWrite(ledPinRouge, LOW);
    time1 = millis();
    while(buttonState == HIGH) { buttonState = digitalRead(buttonPin); }
    time2 = millis();
    timePushed = time2 - time1;
  } 
  if( timePushed > 1000)
  {
      //Allumage de la LED jaune pour indiquer que le mode serveur est activé
      digitalWrite(ledPinJaune, HIGH);
      Serial.println("LED allumee");
      serveur();
      digitalWrite(ledPinJaune, LOW);
  }
  else if(timePushed > 0)
  {
    digitalWrite(ledPinVerte, HIGH);
    Serial.println("LED éteinte");
    Serial.print("Connexion à : ");
    Serial.println(host);
  
    // Utilisation de la classe WiFiClient pour créer une connexion TCP
    WiFiClient client;
  
    //Ecoute sur le port 
    const int httpPort = 80;
    //Vérification de l'état de la connexion au serveur 
    if (!client.connect(host, httpPort))
    {
        Serial.println("Connexion échoué");
        digitalWrite(ledPinVerte, LOW);
        return;
    }
    //Creation d'une URL pour la requête
    String url = "/get";
 
    Serial.print("Requête URL: ");
    Serial.println(url);
    
    //Envoie de la requête au serveur
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0)
    {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
  
    // Lecture de toutes les lignes retourner par le serveur et impression de ces dernière dans le port série
    while(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  
    Serial.println();
    
    Serial.println("Fermeture de la connexion");
    digitalWrite(ledPinVerte, LOW);
  } 
}

void serveur()
{ 
  int out=0;
  while(out == 0)
  {
    server.handleClient(); 
    buttonState = digitalRead(buttonPin);
    if(buttonState == HIGH)
    {
      out =1;
    }
    //Serial.println("SERVER FUNCTION");
  }
}

String FlashConfig()
{
  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();
  String res = "<ul><li>Flash real id :";
  res += ESP.getFlashChipId();
  res += "</li>";
  res += "<li>Flash ide mode : ";
  res += (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN");
  res += "</li>";
  res += "<li>Flash ide  size : ";
  res += ideSize;
  res += "</li>";
  res += "<li>Flash ide speed : ";
  res += ESP.getFlashChipSpeed();
  res += "</li>";
  res += "</ul>";
  
  return res;

}

String WifiScan()
{
  String res;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Indique le nombre de réseaux trouvés
  int n = WiFi.scanNetworks();
  
  if (n == 0)
    return "aucun réseau détecté";
  else
  {
    res +=n ;
    res += " réseaux trouvés";
    res += "<ul>";
    for (int i = 0; i < n; ++i)
    {
      // Affiche le SSID et le RSSI de chaque réseaux trouvés
      res += "<li>";
      res += i + 1;
      res += ": ";
      res += WiFi.SSID(i);
      res += " (";
      res += WiFi.RSSI(i);
      res += ")";
      res += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      res += "</li>";
      delay(10);
    }
    res += "</ul>";

    return res;
  }
  
}

