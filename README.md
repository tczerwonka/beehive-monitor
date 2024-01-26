# hive-monitor

  ESP8266 on NodeMCU board to report temperature from a thermocouple
  and 1wire temp sensor via MQTT to a server
  Voltage monitor on input pin.
  Ultimate use is to monitor bee hive temperature from an embedded K-type
  thermocouple probe along with external temperature.  Load cells can be
  connected to the unit at a future date.

  !(web/20240123_130516~2.jpg)
  !(web/20240123_130532~2.jpg)
  !(web/img_thermal_1706036679313~2.jpg)
  !(web/img_thermal_1706036703662~2.jpg)

  Max power consumption approx 90ma.  Sleep consumption -- about 9.6mA.

  Using a single 18650 cell with a 10W solar cell and cheap mppt charge
  controller to power the hardware.

  Power save not significantly reduced by disconnecting 1wire.  This is
  approximately in-line with the docs.  Guessing a lot of waste in LDO
  regulator circuit and USB interface.


