#include <dht_nonblocking.h>
//#include "dht_nonblocking.h"
//#include <DHT/dht_nonblocking.h>


//www.elegoo.com
//2018.10.25
#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
int pinLluvia=A0;
int valorLluvia=0;


/*
 * Initialize the serial port.
 */
void setup( )
{
  Serial.begin( 9600);
}



/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
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



/*
 * Main program loop.
 */
void loop( )
{
  float temperature;
  float humidity;
  valorLluvia=analogRead(pinLluvia);
  /* Measure temperature and humidity.  If the functions returns
     true, then a measurement is available. */
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T:" );//en grados C
    Serial.print( temperature, 1 );
    Serial.print( ", H:" );//%
    Serial.print( humidity, 1 );
    Serial.print( ", Lluvia:" );
    
    Serial.println(valorLluvia);
  }
  if (Serial.available()) {
    String linea = Serial.readStringUntil('\n');  // Recibe línea de Arduino
    Serial.println(linea);
  }
  //valorLluvia=analogRead(pinLluvia);
  //Serial.println(valorLluvia);
  //delay(1000);
}