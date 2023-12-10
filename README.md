# hive-monitor

  ESP8266 on NodeMCU board to report temperature from a thermocouple
  and 1wire temp sensor via MQTT to a server
  Voltage monitor on input pin.
  Ultimate use is to monitor bee hive temperature from an embedded K-type
  thermocouple probe along with external temperature.  Load cells can be
  connected to the unit at a future date.

  Modification of hardware and software -- added mosfet to switch off the
  the thermocouple board between readings to save power.  If voltage drops
  below 3.7 volts change update time to 5min instead of 60s.

  Max power consumption approx 90ma.  Sleep consumption -- about 9.6mA.

  Power save not significantly reduced by disconnecting 1wire.  This is
  approximately in-line with the docs.  Guessing a lot of waste in LDO
  regulator circuit and USB interface.
