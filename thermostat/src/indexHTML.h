/**
 * @file indexHTML.h
 * @brief This file describes the index page of the webserver.
 * As there is no template engine (à la Jinja), the HTML is split up in a number of pieces
 * to allow for dynamic content generation based on the state of the Thermostat.
 */
#include <Thermostat.h>
#include <ESP8266WebServer.h>

String index_page_html_untill_thermostat_state_button = R""""(
<!DOCTYPE html>
<html>
<head>
<style>
.button {
  border: none;
  color: white;
  padding: 2vw 8vw;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 4vw;
  margin: 4px 2px;
  cursor: pointer;
}

.button1 {background-color: #085c4e;} /* Green */
.button2 {background-color: #65819b;} /* Blue */

p{
    font-size: 4vw;
}
b {
    font-size: 4vw;
}

h1{
    font-size: 6vw;
}

h3 {
    font-size: 5vw;
}

</style>
</head>
<body>
    <center>
        <h1> Thermostat WebServer </h1>
        <h3> current temperature = {{temp}} </h3>
        <br>
        <a href="/switch_thermostat_state">
            <button class="button button1">
                Thermostat {{state}}
            </button>
        </a>

)""""; // requires temp, thermostat_state

String index_page_html_thermostat_state = R""""(
<h3> heater is {{heater_state}}</h3>
<a href="/decrease_target_temperature">
    <button class="button button1">
        -
    </button>
</a>
<b> {{target_temperature}} </b>
<a href="/increase_target_temperature">
    <button class="button button1">
        +
    </button>
</a>        
<p>lower threshold = {{lower_temp_threshold}}<br>
upper threshold = {{upper_temp_threshold}}</p>

)""""; // requires heater_state, target_temperature, lower_temp_threshold , upper_temp_threshold

String index_page_html_closing = R""""(
</center>
</body>
</html>
)"""";

String float2str(float value)
{
    char str_value[6] = "";
    sprintf(str_value, "%.1f", value);
    return String(str_value);
}

String renderIndex(float current_temperature, bool heater_on, Thermostat *thermostat)
/**
 * @brief Renders the index page by 'processing' the HTML template and replacing {{<param>}} fields.
 * 
 */
{
    String content = "";
    content += index_page_html_untill_thermostat_state_button;
    content.replace("{{temp}}", float2str(current_temperature));

    if (thermostat->isActive())
    {
        content.replace("{{state}}", "on");
        content += index_page_html_thermostat_state;
        if (heater_on)
        {
            content.replace("{{heater_state}}", "on");
        }
        else
        {
            content.replace("{{heater_state}}", "off");
        }
        content.replace("{{target_temperature}}", float2str(thermostat->getDesiredTemperature()));
        content.replace("{{lower_temp_threshold}}", float2str(thermostat->getDesiredTemperature() - thermostat->getLowerTemperatureMargin()));
        content.replace("{{upper_temp_threshold}}", float2str(thermostat->getDesiredTemperature() + thermostat->getUpperTemperatureMargin()));
    }
    else
    {
        content.replace("{{state}}", "off");
    }
    content += index_page_html_closing;
    return content;
}