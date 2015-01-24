# Log WiFi Data Usage

Scans for all wifi adapters on the computer and logs the amount of data transmitted and received so far every 30 seconds to a file named after the SSID of the network to which the adapter is connected. The logs are saved in the %PROGRAMDATA%/SDXTECH/DataUsageLogger folder.

Run install.cmd from an elevated command prompt.

Open services.msc and locate the Wifi data usage logger service and change the startup mode to automatic.
