void ConnectWiFi() {
	WiFiManager wifiManager;
	wifiManager.setTimeout(180);
	if (!wifiManager.autoConnect()) {
		// in case of a power outage where we cannot connect to the WiFi temporarely
		// we don't want to be stuck in the config portal forever...
		ESP.reset();
	}
}
