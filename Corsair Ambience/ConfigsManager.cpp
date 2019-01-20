#include "ConfigsManager.h"
#include <iostream>
#include <SimpleIni.h>
#include "DXGIManager.h"

#define settingsFileName "settings.ini"

CSimpleIniA ini;
DXGIManager dxgiManager;

void ConfigsManager::printLoadedSettings() {
	std::cout << "Refresh interval: " << sleepDuration.load() << " milliseconds" << std::endl;
	std::cout << "Horizontal Step: " << stepX << std::endl;
	std::cout << "Vertical Step: " << stepY << std::endl;
	std::cout << "Horizontal Zones: " << horizontalZones << std::endl;
	std::cout << "Vertical Zones: " << verticalZones << std::endl;
	std::cout << "Pinned App to monitor number: " << pinToMonitor << std::endl;
	std::cout << "Check for an update on start up is " << (checkForUpdateFF ? "ON" : "OFF") << std::endl;
	std::cout << "Multi-monitor mode is " << (multiMonitorSupport ? "ON" : "OFF") << std::endl;
	std::cout << "Keyboard zone coloring is " << (keyboardZoneColoring ? "ON" : "OFF") << std::endl;
	std::cout << "Mousepad zone coloring is " << (mousePadZoneColoring ? "ON" : "OFF") << std::endl;
	std::cout << "Filter bad colors is " << (filterBadColors ? "ON" : "OFF") << "\n" << std::endl;
}

bool stringToBool(std::string str) {
	if (str.compare("true") == 0 || str.compare("on") == 0 || str.compare("1") == 0) {
		return true;
	}
	else {
		return false;
	}
}

int getConfigValAsInt(const char* section, const char* key, int defaultVal, int rangeLowerBound, int rangeUpperBound) {
	int res;
	try {
		res = std::stoi(ini.GetValue(section, key), nullptr);
		if (rangeLowerBound && rangeUpperBound && (res < rangeLowerBound || res > rangeUpperBound)) {
			std::cout << "Error: the setting \"" << key << "\" must be between " << rangeLowerBound << " and " << rangeUpperBound << std::endl;
			std::cout << "Using a default value instead: " << defaultVal << "\n" << std::endl;
			res = defaultVal;
		}
	}
	catch (const std::exception&) {
		std::cout << "Failed to read the value of " << key << " from the settings file" << std::endl;
		std::cout << "Using a default value instead: " << defaultVal << "\n" << std::endl;
		res = defaultVal;
	}
	return res;
}



void ConfigsManager::loadConfigsFromSettingsFile() {
	ini.SetUnicode();
	int rc = ini.LoadFile(settingsFileName);
	if (rc < 0) {
		std::cout << "Failed to load the settings file: " << settingsFileName << std::endl;
		return;
	}

	sleepDuration = getConfigValAsInt("CONFIGS", "RefreshInterval", sleepDuration, 15, 1000);

	stepX = getConfigValAsInt("CONFIGS", "HorizontalStep", stepX, 1, 32);
	stepY = getConfigValAsInt("CONFIGS", "VerticalStep", stepY, 1, 32);
	horizontalZones = getConfigValAsInt("CONFIGS", "HorizontalZones", horizontalZones, 1, 25);
	verticalZones = getConfigValAsInt("CONFIGS", "VerticalZones", verticalZones, 1, 25);
	pinToMonitor = getConfigValAsInt("CONFIGS", "PinToMonitor", pinToMonitor, -1, dxgiManager.GetMonitorCount());

	multiMonitorSupport = stringToBool(ini.GetValue("CONFIGS", "MultiMonitorSupport"));
	checkForUpdateFF = stringToBool(ini.GetValue("CONFIGS", "CheckForUpdateOnStartup"));
	keyboardZoneColoring = stringToBool(ini.GetValue("CONFIGS", "KeyboardZoneColoring"));
	mousePadZoneColoring = stringToBool(ini.GetValue("CONFIGS", "MousePadZoneColoring"));
	filterBadColors = stringToBool(ini.GetValue("CONFIGS", "FilterBadColors"));
	DXGI = stringToBool(ini.GetValue("CONFIGS", "DXGI_API"));
}





