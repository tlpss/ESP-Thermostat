#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Thermostat.h"
#include <stdio.h>
#include "config.h"
#include <OneWire.h>
#include <DS18B20.h>

const int LED_PIN = D0;
const int TEMP_SENSOR_PIN = D2;
const int RELAIS_PIN = D3;

// initialize the Thermostat
Thermostat *thermostat = new Thermostat();

// initialize temperature sensor
OneWire oneWire(TEMP_SENSOR_PIN);
DS18B20 temperatureSensor(&oneWire);

// Set web server port number to 80
WiFiServer server(80);
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org");

bool should_read_temperature = false;
bool should_heater_be_turned_on = false;
float most_recent_temperature_measurement;

void temperatureTimerISR(){
  should_read_temperature = 1;
}

void serveWebserver()
{

  float webserverIncrementValue = 0.5;
  char activateThermostatRoute[] = "/status/activate";
  char deactivateThermostatRoute[] = "/status/deactivate";
  char increaseDesiredTemperatureRoute[] = "/desiredTemperature/increase";
  char descreaseDesiredTemperatureRoute[] = "/desiredTemperature/decrease";

  // time bookkeeping for requests
  unsigned long currentTime = millis();
  unsigned long previousTime = 0;
  const long timeoutTime = 2000; // ms

  // Variable to store the HTTP request
  String header;

  // webserver
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  {                                // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf(activateThermostatRoute) >= 0)
            {
              thermostat->activate();
            }
            else if (header.indexOf(deactivateThermostatRoute) >= 0)
            {
              thermostat->deactivate();
            }
            else if (header.indexOf(increaseDesiredTemperatureRoute) >= 0)
            {
              thermostat->setDesiredTemperature(thermostat->getDesiredTemperature() + webserverIncrementValue);
            }
            else if (header.indexOf(descreaseDesiredTemperatureRoute) >= 0)
            {
              thermostat->setDesiredTemperature(thermostat->getDesiredTemperature() - webserverIncrementValue);
                // client.print("<HEAD>");
                // client.print("<meta http-equiv=\"refresh\" content=\"0;url=/\">");
                // client.print("</head>");
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Thermostat Web Server</h1>");

            if (thermostat->isActive())
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "<p><a href=\"%s\"><button class=\"button\">thermostat ON</button></a></p>",deactivateThermostatRoute);
              client.println(buffer);

              client.printf("<p>Current Temperature = %.1f</p> \n",most_recent_temperature_measurement);
              client.printf("<p>Desired Temperature = %.1f</p> \n",thermostat->getDesiredTemperature());

            //have increase and decrease buttons
              snprintf(buffer, sizeof(buffer), "<p><a href=\"%s\"><button class=\"button\">increase desired temperature</button></a></p>",increaseDesiredTemperatureRoute);            
              client.println(buffer);

              snprintf(buffer, sizeof(buffer), "<p><a href=\"%s\"><button class=\"button\">decrease desired temperature</button></a></p>",descreaseDesiredTemperatureRoute);
              client.println(buffer);
              if (should_heater_be_turned_on){
                client.println("<b>heater is on</b>");
              }
              else {
                client.println("<b> heater is off </b>");
              }
              client.printf("<p>lower threshold = %.1f; upper threshold = %.1f</p> \n", thermostat->getDesiredTemperature()-thermostat->getLowerTemperatureMargin(),thermostat->getDesiredTemperature() + thermostat->getUpperTemperatureMargin());

            }
            else
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "<p><a href=\"%s\"><button class=\"button button2\">thermostat OFF</button></a></p>",activateThermostatRoute);
              client.println(buffer);
            
            }
            client.println();
            /* have increase and decrease buttons
            <div class="action_btn">

            <button name="submit" class="action_btn submit" type="submit" value="Save" onclick="myFunction()">Save</button>
            <button name="submit" class="action_btn cancel" type="submit" value="Cancel" onclick="myFunction2()">Cancel</button>

            </div>
            */

            client.println("</body></html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void setup()
{

  Serial.begin(115200);

  // IO pin setup 
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAIS_PIN, OUTPUT);


  // setup timer interrupts 
  timer1_attachInterrupt(temperatureTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE,TIM_LOOP);
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
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  IPAddress localIP = WiFi.localIP();
  Serial.println(localIP);
  server.begin();

  // start NTP client
  ntpClient.begin();
  ntpClient.setTimeOffset(3600); //TODO: deal with zomeruur/winteruur 
  ntpClient.setUpdateInterval(10000);

  // start temperature sensor
    // wait until address found
  if (temperatureSensor.begin() == false)
  {
    Serial.println("ERROR: No device found");
    while (!temperatureSensor.begin()); // wait until device comes available.
  }

  temperatureSensor.setResolution(12);
  temperatureSensor.setConfig(DS18B20_CRC);  // or 1
  temperatureSensor.requestTemperatures();
}


float readTemperatureSensor(DS18B20 sensor){
  uint32_t start = millis();
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

  if (should_read_temperature){

    should_read_temperature = 0;

    // read temperature
    most_recent_temperature_measurement = readTemperatureSensor(temperatureSensor);
    // decide if heater should be turned on
    should_heater_be_turned_on = thermostat->shouldHeatBeOn(most_recent_temperature_measurement, should_heater_be_turned_on);

    // actuate relais
    Serial.printf("should heater be on = %d \n",should_heater_be_turned_on);
    digitalWrite(RELAIS_PIN, should_heater_be_turned_on);
  }


  serveWebserver();
}
