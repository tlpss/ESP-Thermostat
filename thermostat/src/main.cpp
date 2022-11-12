/***
 * main.cpp file that creates a Thermostat and webserver for interacting with the Thermostat.
 * The thermostat is used to operate a Relais that is connected to the central heating system.
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include "config.h"
#include <OneWire.h>
#include <DS18B20.h>

#include "Thermostat.h"
#include "indexHTML.h"

const int LED_PIN = D0;
const int TEMP_SENSOR_PIN = D2;
const int RELAIS_PIN = D3;

// webserver
// initialize the Thermostat
Thermostat *thermostat = new Thermostat();

// initialize temperature sensor
OneWire oneWire(TEMP_SENSOR_PIN);
DS18B20 temperatureSensor(&oneWire);

// initialize web server
ESP8266WebServer server(80);

// initialize routes (these should match the routes in the webServer.h HTML) and other UI variables.
float webserverIncrementValue = 0.5;
char switchThermostatStateRoute[] = "/switch_thermostat_state";
char increaseDesiredTemperatureRoute[] = "/increase_target_temperature";
char descreaseDesiredTemperatureRoute[] = "/decrease_target_temperature";

// create NTP client
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org");

// state variables and flags
bool should_read_temperature = false;
bool should_heater_be_turned_on = false;
float most_recent_temperature_measurement;

void temperatureTimerISR()
{
  /**
   * @brief interrupt service routine for temperature timer
   */
  should_read_temperature = 1;
}

void setupServerRouting()
{
  server.on("/", []()
            { server.send(200, "text/html", renderIndex(most_recent_temperature_measurement, should_heater_be_turned_on, thermostat)); });
  server.on(switchThermostatStateRoute, []()
            { 
    if (thermostat->isActive()){
      thermostat->deactivate();
    }
    else{
       thermostat->activate();
    }
    // redirect to index page
    // to avoid retriggering the request
    server.sendHeader("Location","/",true);
    // 303 is for redirect after POST, but for sake of simplicity all requests are GET
    server.send(303,"text/plain",""); });
  server.on(increaseDesiredTemperatureRoute, []()
            {
              thermostat->setDesiredTemperature(thermostat->getDesiredTemperature() + webserverIncrementValue);
              server.sendHeader("Location", "/", true);
              server.send(303, "text/plain", ""); });
  server.on(descreaseDesiredTemperatureRoute, []()
            {
    thermostat->setDesiredTemperature(thermostat->getDesiredTemperature() - webserverIncrementValue);
    server.sendHeader("Location","/",true);
    server.send(303,"text/plain",""); });
}

void setup()
{

  Serial.begin(115200);

  // IO pin setup
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAIS_PIN, OUTPUT);

  // setup timer interrupts
  timer1_attachInterrupt(temperatureTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  timer1_write(5000000 * 5); // every 5 seconds;

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(CONFIG_SSID);
  WiFi.begin(CONFIG_SSID, CONFIG_WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  IPAddress localIP = WiFi.localIP();
  Serial.println(localIP);

  // start NTP client
  ntpClient.begin();
  ntpClient.setTimeOffset(3600); // TODO: deal with zomeruur/winteruur
  ntpClient.setUpdateInterval(10000);

  // start temperature sensor
  // wait until address found
  if (temperatureSensor.begin() == false)
  {
    Serial.println("ERROR: No device found");
    while (!temperatureSensor.begin())
      ; // wait until device comes available.
  }

  temperatureSensor.setResolution(12);
  temperatureSensor.setConfig(DS18B20_CRC); // or 1
  temperatureSensor.requestTemperatures();

  // setup server
  server.begin();
  setupServerRouting();
}

float readTemperatureSensor(DS18B20 sensor)
{
  sensor.requestTemperatures();

  // wait for data AND detect disconnect
  uint32_t timeout = millis();
  while (!sensor.isConversionComplete())
  {
    if (millis() - timeout >= 800) // check for timeout
    {
      Serial.println("ERROR: timeout or disconnect");
      break;
    }
  }

  float temperature = sensor.getTempC();

  if (temperature == DEVICE_CRC_ERROR)
  {
    Serial.println("ERROR: CRC error");
    return -22.22;
  }
  return temperature;
}
void loop()
{

  ntpClient.update();

  digitalWrite(LED_PIN, 1 - thermostat->isActive());

  // read the current temperature and
  // decide if heater should be turned on

  if (should_read_temperature)
  {

    should_read_temperature = 0;

    // read temperature
    most_recent_temperature_measurement = readTemperatureSensor(temperatureSensor);
    // decide if heater should be turned on
    should_heater_be_turned_on = thermostat->shouldHeatBeOn(most_recent_temperature_measurement, should_heater_be_turned_on);

    // actuate relais
    Serial.printf("should heater be on = %d \n", should_heater_be_turned_on);
    digitalWrite(RELAIS_PIN, should_heater_be_turned_on);
  }

  server.handleClient();
}
