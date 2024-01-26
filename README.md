# hive-monitor

  ESP8266 on NodeMCU board to report temperature from a thermocouple
  and 1wire temp sensor via MQTT to my local stats server.  Data visualization
  via grafana.
  Voltage monitor on input pin.

  Ultimate use is to monitor bee hive temperature from an embedded K-type
  thermocouple probe along with external temperature.  Load cells can be
  connected to the unit at a future date.

  2day temperature graph

  !(https://raw.githubusercontent.com/tczerwonka/beehive-monitor/master/web/grafana-temp2dy.png)

  7day temperature graph

  !(https://raw.githubusercontent.com/tczerwonka/beehive-monitor/master/web/grafana-temp7dy.png)

  [hive front, photo](https://raw.githubusercontent.com/tczerwonka/beehive-monitor/master/web/20240123_130532%7E2.jpg)

  [hive front, thermal image](https://github.com/tczerwonka/beehive-monitor/blob/master/web/img_thermal_1706036703662~2.jpg)

  [hive side, photo](https://raw.githubusercontent.com/tczerwonka/beehive-monitor/master/web/20240123_130516%7E2.jpg)

  [hive side, thermal](https://github.com/tczerwonka/beehive-monitor/blob/master/web/img_thermal_1706036679313~2.jpg)

  Max power consumption approx 90ma.  Sleep consumption -- about 9.6mA.

  Using a single 18650 cell with a 10W solar cell and cheap mppt charge
  controller to power the hardware.

  Power save not significantly reduced by disconnecting 1wire.  This is
  approximately in-line with the docs.  Guessing a lot of waste in LDO
  regulator circuit and USB interface.


