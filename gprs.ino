#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"

// for GPRS
InetGSM inet;
char msg[50];
boolean started=false;
unsigned long lastRequest = 0;
long interval = 15000; //15 seconds
char itoaBuffer[20];

void setup() 
{
  //Serial connection.
  Serial.begin(9600);
  Serial.println("GSM Shield testing.");
  //Start configuration of shield with baudrate.
  //For http uses is raccomanded to use 4800 or slower.
  if (gsm.begin(2400)){
    Serial.println("\nstatus=READY");
    started=true;  
  }
  else Serial.println("\nstatus=IDLE");

  if(started){
    //GPRS attach, put in order APN, username and password.
    //If no needed auth let them blank.
    if (inet.attachGPRS("internet.wind", "", ""))
      Serial.println("status=ATTACHED");
    else Serial.println("status=ERROR");
    delay(1000);

    //Read IP address.
    gsm.SimpleWriteln("AT+CIFSR");
    delay(5000);
    //Read until serial buffer is empty.
    gsm.WhileSimpleRead();
  }
}

void loop() 
{
  unsigned long time = millis();
  if(time - lastRequest > interval)
  {
    double HCHO = getHCHO();
    sprintf(itoaBuffer, "{\"value\":%d.%04d}", ceil(HCHO), ceil(10000*(HCHO-ceil(HCHO))));
    Serial.print("value: ");
    Serial.println(itoaBuffer);
    inet.httpPOST("api.yeelink.net",
    80,
    "/v1.0/device/449/sensor/3912/datapoints",
    itoaBuffer,
    msg,
    50,
    "U-ApiKey: 268d326bba82405fbffxxxxxxxxxxxxx");
    lastRequest = time;
  }
}

double getHCHO()
{
  //AD convertor data
  int val = analogRead(A0);
  double V = 5.0*val/1024.0;

  //ppm
  double ppm = 0.5*V/0.3765;

  // mg/m3
  double mgm3 = 30.0*ppm/22.4;

  return mgm3;
}
