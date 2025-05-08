#include <dht_nonblocking.h>
#include <LiquidCrystal.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
int pinLluvia = A0;
int valorLluvia = 0;

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Scroll por línea
String mensajesScroll[2] = {"", ""};
unsigned long ultimaActualizacionScroll[2] = {0, 0};
int indiceScroll[2] = {0, 0};
bool scrollActivo[2] = {false, false};

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Inicializando");
}

static bool measure_environment(float *temperature, float *humidity) {
  static unsigned long measurement_timestamp = millis();

  if (millis() - measurement_timestamp > 3000ul) {
    if (dht_sensor.measure(temperature, humidity) == true) {
      measurement_timestamp = millis();
      return true;
    }
  }

  return false;
}

void loop() {
  float temperature;
  float humidity;
  valorLluvia = analogRead(pinLluvia);

  if (measure_environment(&temperature, &humidity) == true) {
    Serial.print("T:");
    Serial.print(temperature, 1);
    Serial.print(", H:");
    Serial.print(humidity, 1);
    Serial.print(", Lluvia:");
    Serial.println(valorLluvia);
  }

if (Serial.available()) {
  String recibido = Serial.readStringUntil('\n');
  int fila = 0;

  if (recibido.startsWith("Servidor: ")) {
    fila = 1;
  }

  // Evitar reiniciar scroll si el mensaje es igual
  if (recibido != mensajesScroll[fila]) {
    mensajesScroll[fila] = recibido;
    scrollActivo[fila] = recibido.length() > 16;
    indiceScroll[fila] = 0;
    ultimaActualizacionScroll[fila] = millis();

    lcd.setCursor(0, fila);
    if (!scrollActivo[fila]) {
      lcd.print(recibido + "                "); // limpiar resto de línea
    }
  }
}


  // Scroll no bloqueante por línea
for (int fila = 0; fila < 2; fila++) {
  if (scrollActivo[fila] && millis() - ultimaActualizacionScroll[fila] > 300) {
    lcd.setCursor(0, fila);
    
    // Asegurar que siempre se impriman 16 caracteres (relleno con espacios)
    String fragmento = mensajesScroll[fila].substring(indiceScroll[fila], indiceScroll[fila] + 16);
    while (fragmento.length() < 16) {
      fragmento += " ";
    }

    lcd.print(fragmento);
    indiceScroll[fila]++;
    ultimaActualizacionScroll[fila] = millis();

    // Reiniciar scroll cuando se llegue al final
    if (indiceScroll[fila] > mensajesScroll[fila].length() - 16) {
      indiceScroll[fila] = 0;
    }
  }
}

}
