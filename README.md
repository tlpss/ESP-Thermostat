# ESP-Thermostat

This repo implements a standalone bang-bang thermostat for controlling room temperature for a central heating system. The thermostat actuates a relais
that operates the boiler. The thermostat has a minimal web interface for convenience and uses configurable hysteresis intervals to reduce the number of boiler triggers.


## Hardware
### Components
- [NodeMCU ESP8266]()
- [DS18B20](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf) Temperature sensor
- [SRD-5VDC-SL-C](https://www.circuitbasics.com/wp-content/uploads/2015/11/SRD-05VDC-SL-C-Datasheet.pdf) 5V Relay.
- some resistors, cables and breadboard or prototyping board. 

### Wiring
- plug a 5V adapter to the NodeMCU (keep in mind that the operating voltage is 3.3V!)
- connect relay to the boiler circuit (ues the normal open and comm pins).
- connect the relay to the D3 pin of the NodeMCU, and the V_in of the NodeMCU. Do not use a 3.3V pin as this will result in faulty behaviour of the relay.
- connect the sensor to the V_in port of the NodeMCU (3.3V is supposed to work but was less stable for me), connect the data pin to the D2 pin of the NodeMCU and add a pull-up resistor as described in the data sheet.
You can see an example layout (with additional LED) below:
![IMG20221112232500](https://user-images.githubusercontent.com/37955681/201497066-e70f6cb0-708e-4670-8a83-87853e2a5715.jpg)

## Software
- install Platformio for vscode (instructions [here](https://platformio.org/install/ide?install=vscode))
- install the [Espressif Platform](https://docs.platformio.org/en/stable/platforms/espressif8266.html#platform-espressif8266) 
- clone this repo and open the platformio project
- upload the code after connecting the ESP with your pc.

### Web Interface
To interact with the Thermostat a minimal web interface is available. With this interface you can read the current temperature, turn the thermostat on/off and set the desired temperature.
To find the Thermostat's IP address you should use the serial monitor after uploading the program, the IP address will be printed.

You can see the interface below:

![signal-2022-11-12-233022 (1)](https://user-images.githubusercontent.com/37955681/201521819-dd708d33-c06b-4e65-b0bc-bcdd457535bb.jpeg)


Note that there is no authentication for the web interface, so you should not expose it outside of your local network.

## Note on power efficiency
- This Thermostat requires an 5V power supply and is far from power-efficient due to the constant polling of the webserver, best to keep it plugged in.
If you want to make this more efficient, you could use an IoT protocol such as [MQTT](https://mqtt.org/) that e.g. pulls user input every 10s instead of creating a webserver that is constantly polling for client connections. Note however that in that case you need a seperate device that creates the UI (a raspberry Pi for example) that functions as MQTT broker and that runs a webserver. On this device however, you can just use standard web development frameworks instead of having to hack something on an embedded device in C++.


## Resources
- [esp webserver tutorial](https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html)
- [temperature sensor library](https://github.com/matmunk/DS18B20)
