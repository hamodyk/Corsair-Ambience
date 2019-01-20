#pragma once
#include <atomic>
#include <string>

class ConfigsManager {
public:
	void printLoadedSettings();
	void loadConfigsFromSettingsFile();

private:

	
public:
	int ScreenX = 0;
	int ScreenY = 0;
	int stepX = 5;
	int stepY = 5;
	int horizontalZones = 17;
	int verticalZones = 6;
	int pinToMonitor = -1;

	std::atomic_bool continueExecution{ true };
	std::atomic_bool multiMonitorSupport{ true };
	std::atomic_int sleepDuration{ 500 };
	bool checkForUpdateFF = true;
	bool keyboardZoneColoring = true;
	bool mousePadZoneColoring = true;
	bool filterBadColors = true;
	bool DXGI = true;
	bool screenSizeChanged = false;
};