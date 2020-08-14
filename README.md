IR Example code to send a single command to a TV or similar device from a simple web-server command.

Code made for a WeMo D1 Mini (ESP8266). 

Use the IRremoteESP8266 example IRrecvDumpV3 to get the command from your remote first (be sure to change the pin or connect to the pin inthe example). 

Note, due to the current required for the IR LED, it's highly suggested to power this from an external battery vs. from the USB. A USB power-brick that provides at least 1A/5V would work - this is not provided by a standard PC connection. See bread-board for a basic LM7805 regulator to use a 9V battery.
