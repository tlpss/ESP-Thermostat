#ifndef THERMOSTAT_H
#define THERMOSTAT_H

class Thermostat
{
    /// @brief Holds the state for an On-Off Thermostat. All values are in degrees Celcius.
private:
    bool _isActive = false;
    float desiredTemperature = 19.0;
    float lowerTemperatureMargin = 1.0;
    float upperTemperatureMargin = 1.0;

    bool isPlausibleTemperature(float);
    bool isPlausibleMargin(float);

public:
    bool isActive();
    void activate();
    void deactivate();
    bool shouldHeatBeOn(float,bool);
    float getDesiredTemperature();
    int setDesiredTemperature(float);
    float getLowerTemperatureMargin();
    int setLowerTemperatureMargin(float);
    float getUpperTemperatureMargin();
    int setUpperTemperatureMargin(float);
};

#endif