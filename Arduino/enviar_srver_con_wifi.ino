#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h> // Librería de WiFiManager
#include <EEPROM.h>

#define EEPROM_SIZE 100
String servidor;

// Función para guardar la IP del servidor en EEPROM
void guardarServidor(String ip) {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < ip.length(); ++i) {
    EEPROM.write(i, ip[i]);
  }
  EEPROM.write(ip.length(), '\0');  // Fin de cadena
  EEPROM.commit();
}

// Función para leer la IP del servidor desde EEPROM
String leerServidor() {
  EEPROM.begin(EEPROM_SIZE);
  char ip[100];
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    ip[i] = EEPROM.read(i);
    if (ip[i] == '\0') break;
  }
  return String(ip);
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(EEPROM_SIZE);

  WiFiManager wifiManager;

  // Campo personalizado para la IP del servidor
  WiFiManagerParameter custom_server("server", "URL del servidor", leerServidor().c_str(), 100);

  wifiManager.addParameter(&custom_server);

  // Inicia el portal de configuración si no hay red guardada
  wifiManager.startConfigPortal("ESP_Config");

  // Guarda y muestra el valor del servidor
  servidor = String(custom_server.getValue());
  guardarServidor(servidor);
  Serial.println("Conectado a la red: " + WiFi.SSID());
  Serial.println("Servidor configurado: " + servidor);
} 

void loop() {
  if (Serial.available()) {
    String linea = Serial.readStringUntil('\n');  // Recibe línea de Arduino

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      WiFiClient client;

      http.begin(client, servidor);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String datos = "dato=" + linea;
      http.POST(datos);
      http.end();
    }
  }

  Serial.println("Red: " + WiFi.SSID());
  Serial.println("Servidor: " + servidor);
}
