#ifndef THERMOSTAT_H
#define THERMOSTAT_H

class Thermostat
{
    /// @brief Holds the state for a Bang-Bang Thermostat with Hysteresis. All values are in degrees Celcius.
    /*
    behavior:
    idea is to avoid switching off and on too frequently (most heating devices will have a protection against this but still..)
    so have an 'interval' of temperatures instead of tracking an exact temperature.

    when heating:
        - if temp > desired + uppermargin: turn off
        - else: keep heating
    when not heating:
        if temp < desired - lowermargin: start heating
        else: keep off

    keep in mind that there will be residual heat dissipation afther the CV 
    turns of, this depends on the state of the radiator valves, so unless you known this,
    you cannot really take this into account. Best you could do is register an average, by e.g.
    measuring how much the temperature keeps increasing after you turn the thermostat off on average
    and subtracting this from the upper limit.

    But a proxy is to just set the upper margin to 0.
    */

private:
    bool _isActive = false;
    float desiredTemperature = 19.0;
    float lowerTemperatureMargin = 1.0;
    float upperTemperatureMargin = 0.0; 

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