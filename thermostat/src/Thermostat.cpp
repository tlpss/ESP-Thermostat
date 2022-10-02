#include "Thermostat.h"

bool Thermostat::isActive(){
    return _isActive;
}
void Thermostat::activate(){
    _isActive = true;
}

void Thermostat::deactivate(){
    _isActive = false;
}
bool Thermostat::isPlausibleMargin(float margin){
    return (0.0 <= margin && margin <= 3.0);
}
bool Thermostat::isPlausibleTemperature(float temperature)
{
    return (5.0 <= temperature && temperature <= 35.0);
}



bool Thermostat::shouldHeatBeOn(float temperature,bool heatCurrentlyOn)
{
    if (!isPlausibleTemperature(temperature))
    {
        return false;
    }

    if (!this->isActive()){
        return false;
    }

    bool shouldOn = (temperature <= this->desiredTemperature - this->lowerTemperatureMargin) && ! heatCurrentlyOn; // should truly start
    shouldOn = shouldOn || ((temperature <= this->desiredTemperature + this->upperTemperatureMargin) && heatCurrentlyOn); // still heating
    return shouldOn;
}

float Thermostat::getDesiredTemperature()
{
    return desiredTemperature;
}

int Thermostat::setDesiredTemperature(float temperature)
{
    if (isPlausibleTemperature(temperature))
    {
        desiredTemperature = temperature;
        return 0;
    }
    return -1;
}

float Thermostat::getLowerTemperatureMargin(){
    return lowerTemperatureMargin;
}

int Thermostat::setLowerTemperatureMargin(float margin){
    if(isPlausibleMargin(margin)){
        lowerTemperatureMargin = margin;
        return 0;
    }
    return -1;
}
float Thermostat::getUpperTemperatureMargin(){
    return upperTemperatureMargin;
}

int Thermostat::setUpperTemperatureMargin(float margin){
    if(isPlausibleMargin(margin)){
        upperTemperatureMargin = margin;
        return 0;
    }
    return -1;
}
