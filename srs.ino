#define BLYNK_PRINT Serial
#include <OneWire.h>
#include <SPI.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define BLYNK_AUTH_TOKEN "PVAbHBBOAE5hbf155x8eKtYwDe-rYK7r" 
#define BLYNK_TEMPLATE_ID "TMPL3R1qoFznv"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation 2"

String apiKey = "QBQD300BUPIDHJTG";
const char* ssid = "KaranOnePlus"; 
const char* pass = "9765619602";
const char* server = "api.thingspeak.com";
WiFiClient client;

int relaypin = D6;
#define DHTPIN 2
#define DHTTYPE DHT11

int moist = 10;
int relayPin = D6;
float h = 0.0;
float t = 0.0;


DHT dht(DHTPIN, DHTTYPE);

void sendSensor()
{
  h = dht.readHumidity();  // Humidity
  t = dht.readTemperature(); // Temperature
  if (isnan(h) || isnan(t)) 
  {
    Serial.println( "Failed to read from DHT sensor! "); 
    return;
  }
}
void setup()
{
  pinMode(relayPin, OUTPUT);
  Serial.begin(9600); 
  dht.begin();
  sensors.begin();
  Serial.println("Connecting to WiFi: " + String(ssid));
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(relayPin, HIGH);
}
int sensor=0; 

int sendTemps()
{
  sensor=analogRead(A0); 
  int output=(145-map(sensor,0,1023,0,140));  // Moisture
  sendSensor();
  Serial.println("moisture =");
  Serial.println(output);
  Serial.println( "% ");
  return output;
}

int moisture() 
{ 
  int output=sendTemps();
  int rainSensor = analogRead(D1);
  rainSensor = map(rainSensor, 0, 1023, 0, 100); // Rain
  Serial.print(rainSensor);
  Serial.println(output);
  Serial.println("Rainfall :");
  if(rainSensor==0){
    Serial.println("Rainfall not detected");
  }
  else{
    Serial.println("Rainfall detected");
  }
  return rainSensor;
}
void check(){
  int rainSensor=moisture();
  int output=sendTemps();  

  Serial.println(rainSensor);
  Serial.println(output);
  sendSensor();

  if(rainSensor==0)
  {
    Serial.println("Motor Stop");
    digitalWrite(relayPin, HIGH);
  }
  else
  {
    digitalWrite(relayPin, LOW);
    if(output<10)
    {
    Serial.println("Motor Start");
      digitalWrite(relayPin, LOW);
    }
    else{
      Serial.println("Motor Stop");
      digitalWrite(relayPin, HIGH);
    }
  }

}
void loop()
{

  int rainSensor=moisture();
  int output=sendTemps();  
  check();
  // sendSensor();
  
  if (client.connect(server, 80)) {
    String postStr = "api_key=" + apiKey + "&field1=" + String(t) + "&field2=" + String(h) + "&field3=" + String(output) + "&field4=" + String(rainSensor) + "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    Serial.print(postStr);
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    client.stop();
    Serial.println("Waiting...");
    // ThingSpeak requires a minimum 15-second delay between updates
    delay(500);
  }
  //digitalWrite(relayPin, HIGH);
}
