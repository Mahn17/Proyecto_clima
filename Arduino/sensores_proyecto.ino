#include <dht_nonblocking.h>
//#include "dht_nonblocking.h"
//#include <DHT/dht_nonblocking.h>
#include <LiquidCrystal.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
int pinLluvia=A0;
int valorLluvia=0;

LiquidCrystal lcd(7,8,9,10,11,12);

void setup( )
{
  Serial.begin( 9600);
  lcd.begin(16,2);
  //lcd.setCursor(0,0);
  lcd.print("Iniciando...");
}


static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  if( millis( ) - measurement_timestamp > 3000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}


void loop( )
{
  float temperature;
  float humidity;
  valorLluvia=analogRead(pinLluvia);
  
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T:" );//en grados C
    Serial.print( temperature, 1 );
    Serial.print( ", H:" );//%
    Serial.print( humidity, 1 );
    Serial.print( ", LLuvia:" );
    Serial.println(valorLluvia);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C H:");
    lcd.print(humidity, 1);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("Lluvia:");
    lcd.print(valorLluvia);
    
  }
  if (Serial.available()) {
    String linea = Serial.readStringUntil('\n');  // Recibe línea de Arduino
    Serial.println(linea);

  }
  //valorLluvia=analogRead(pinLluvia);
  //Serial.println(valorLluvia);
  //delay(1000);
}