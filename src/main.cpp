#include "DHT.h"

#define DHTPIN 4   
#define greenLED 21
#define redLED 27

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

class OperatingMode{
  public:
    bool selectable;
    float minTemp;
    float maxTemp;
    float minHum;
    float maxHum;
};

class ColdFrame {
  public:
    OperatingMode mode;
    bool envOk;
};

ColdFrame frame;

OperatingMode Winter;
OperatingMode Germinating;
OperatingMode Vegetative;
OperatingMode Fruiting;
OperatingMode Night;

void assignOperatingModeVals();
void setEnvStatus();
void setLedStatus(bool envOk);

void assignOperatingModeVals()
{
  Winter.minTemp = 0;
  Winter.maxTemp = 999;
  Winter.minHum = -1;
  Winter.maxHum = 999;

  Germinating.minTemp = 18;
  Germinating.maxTemp = 32;
  Germinating.minHum = 70;
  Germinating.maxHum = 95;

  Vegetative.minTemp = 20;
  Vegetative.maxTemp = 25;
  Vegetative.minHum = 60;
  Vegetative.maxHum = 70;

  Fruiting.minTemp = -1;
  Fruiting.maxTemp = 28;
  Fruiting.minHum = 40;
  Fruiting.maxHum = 50;

  Night.minTemp = 10;
  Night.maxTemp = 15;
  Night.minHum = 30;
  Night.maxHum = 80;
}

void setLedStatus(bool envStat)
{
  if(envStat != frame.envOk)
  {
    if(envStat)
    {
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
    }
    else
    {
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
    }
    frame.envOk = envStat;
  }


}

void setup() {
  Serial.begin(115200);
  while(!Serial);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  assignOperatingModeVals();
  frame.mode = Vegetative;
  dht.begin();
}

void loop() {
  delay(1000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor"));
    delay(5000);
    return;
  }
  setLedStatus(true);
  // // Compute heat index in Fahrenheit (the default)
  // float hif = dht.computeHeatIndex(f, h);
  // // Compute heat index in Celsius (isFahreheit = false)
  // float hic = dht.computeHeatIndex(t, h, false);

  if(t > frame.mode.maxTemp || t < frame.mode.minTemp || h > frame.mode.maxHum || h < frame.mode.minHum)
  {
    setLedStatus(false);
  }
  else
  {
    setLedStatus(true);
  }


}