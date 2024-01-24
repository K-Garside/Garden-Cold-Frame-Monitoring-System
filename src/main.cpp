#include "DHT.h"
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <LiquidCrystal.h>

#define DHTPIN 4   
#define greenLED 21
#define redLED 27

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal LCD(13, 12, 14, 27, 26, 25);

// Assign my epochs and there last changes

const unsigned long DELETION_DELAY = 86400000;
const unsigned long RECORDING_DELAY = 60000;
unsigned long RECORDING_LAST;
const unsigned long SENSOR_DELAY = 1000;
unsigned long SENSOR_LAST;
const unsigned long PRINT_DELAY = 5000;
unsigned long PRINT_LAST;

bool timeDiff(unsigned long start, unsigned long delay)
{
  return (millis() - start) >= delay;
}

class OperatingMode{
  public:
    bool selectable;
    float minTemp;
    float maxTemp;
    float minHum;
    float maxHum;
};

class Reading {
  private:
    float temp;
    float hum;
    unsigned long readingTime;
    OperatingMode mode;
    bool status;
  public:
    Reading(bool status, OperatingMode mode)
    {
      this->temp = dht.readTemperature();
      this->hum = dht.readHumidity();
      this->readingTime = millis();
      this->mode = mode;
      this->status = status;
    }

    bool checkDelete()
    {
      if(timeDiff(this->readingTime, DELETION_DELAY))
      {
        return true;
      }
      else
      {
        return false;
      }
    }
};

class ColdFrame {
  private:
    float currentTemp;
    float currentHum;
    OperatingMode mode;
    bool envOk;
    std::vector<Reading> readings;
  public:
    void setTemp(float temp)
    {
      this->currentTemp = temp;
    }
    float getTemp()
    {
      return this->currentTemp;
    }
    void setHum(float hum)
    {
      this->currentHum = hum;
    }
    float getHum()
    {
      return this->currentHum;
    }
    void setOperatingMode(OperatingMode mode)
    {
      this->mode = mode;
    }
    OperatingMode getOperatingMode()
    {
      return this->mode;
    }
    void setEnvStatus(bool envOk)
    {
      this->envOk = envOk;
    }

    bool getEnvStatus()
    {
      return this->envOk;
    }
    void setLedStatus(bool envStat)
    {
        if(envStat != this->envOk)
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
          this->envOk = envStat;
        }
    }

    void checkReadings()
    {
      for(int i = 0; i < this->readings.size(); i++)
      {
        if(this->readings[i].checkDelete())
        {
          this->readings.erase(this->readings.begin() + i);
        }
      }
    }

    void addReading()
    {
      checkReadings();
      this->readings.push_back(Reading(this->envOk, this->mode));
    }
};

ColdFrame frame;

OperatingMode Winter;
OperatingMode Germinating;
OperatingMode Vegetative;
OperatingMode Fruiting;
OperatingMode Night;

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

void readDHT()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  frame.setTemp(t);
  frame.setHum(h);

  if(t > frame.getOperatingMode().maxTemp || t < frame.getOperatingMode().minTemp || h > frame.getOperatingMode().maxHum || h < frame.getOperatingMode().minHum)
  {
    frame.setLedStatus(false);
  }
  else
  {
    frame.setLedStatus(true);
  }
}

void printDHT()
{
  Serial.print("Current Temp: ");
  Serial.println(frame.getTemp());
  Serial.print("Current Hum: ");
  Serial.println(frame.getHum());

  LCD.clear();

  LCD.setCursor(0, 0);

  LCD.print("Temp: " );
  LCD.print(frame.getTemp());

  LCD.setCursor(0, 1);

  LCD.print("Humi: ");
  LCD.print(frame.getHum());
}

void setup() 
{
  Serial.begin(115200);
  while(!Serial);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  assignOperatingModeVals();
  frame.setOperatingMode(Vegetative);

  LCD.begin(16, 2);

  dht.begin();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor"));
    LCD.print("DHT failed");
    return;
  }
  else
  {
    LCD.clear();

    LCD.setCursor(0, 0);

    LCD.print("Temp: " );
    LCD.print(t);

    LCD.setCursor(0, 1);

    LCD.print("Humi: ");
    LCD.print(h);
  }
}

void loop()
{
  if(timeDiff(SENSOR_LAST, SENSOR_DELAY))//read sensors -- Might not need to delay this, but just in case
  {
    SENSOR_LAST = millis();
    readDHT();
  }
  if(timeDiff(RECORDING_LAST, RECORDING_DELAY))//add new record and delete old ones if necessary. Maybe just check to delete old ones every 24hrs instead rather than every time i add a new one, however this also ensures im not adding more than there is volatile memory.
  {
    RECORDING_LAST = millis();
    frame.addReading();
  }
  if(timeDiff(PRINT_LAST, PRINT_DELAY))//print sensor readings
  {
    PRINT_LAST = millis();
    printDHT();
  }
}