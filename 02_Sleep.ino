long sleepStartTime;
long sleepInterval;

void SetupSleep() {
	sleepStartTime = 0;
	sleepInterval = atoi(INTERVAL_IN_SECONDS) * 1000;
}

void GotoSleep() {
	long executionTime = (millis() - sleepStartTime);
	Serial.printf("Loop execution time (ms): %d.\n", executionTime);

	Serial.printf("Measuring interval (ms): %d.\n", sleepInterval);

	long sleepTime = (sleepInterval - executionTime);
	
	if (sleepTime >= 1 && sleepTime <= sleepInterval) {
		Serial.printf("Going to deep sleep for (ms): %d.\n", sleepTime);
		ESP.deepSleep(sleepTime * 1000, WAKE_RF_DEFAULT);
	}

	sleepStartTime = millis();
}