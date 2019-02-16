#ifdef __APPLE__
#include <CUESDK/CUESDK.h>
#else
#include <CUESDK.h>
#endif


#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <windows.h>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <math.h>
#include "DXGIManager.h"
#include "ConfigsManager.h"
#include "UpdateManager.h"
#include "stdafx.h"

struct RGB { short red; short green; short blue; };
struct Zone { int zoneX; int zoneY; };

ConfigsManager configsManager;

BYTE* ScreenData = 0;


struct postitionComparator {
	bool operator() (const CorsairLedPosition& led1, const CorsairLedPosition& led2) const{
		if (led1.left < led2.left) {
			return true;
		}
		else if (led1.left == led2.left && led1.top < led2.top) {
			return true;
		}
		return false;
	}
};

struct zoneComparator {
	bool operator() (const Zone& zone1, const Zone& zone2) const {
		if (zone1.zoneX < zone2.zoneX) {
			return true;
		}
		else if (zone1.zoneX == zone2.zoneX && zone1.zoneY < zone2.zoneY) {
			return true;
		}
		return false;
	}
};

std::unordered_map<CorsairDeviceType, std::map<CorsairLedPosition, Zone, postitionComparator>> deviceLedsToZonesMap;
std::unordered_map<CorsairDeviceType, std::set<CorsairLedPosition, postitionComparator>> deviceToLedPositionsMap;
std::map<CorsairLedPosition, Zone, postitionComparator> ledPosToZoneMap;
std::map<Zone, RGB, zoneComparator> zoneToRGBmap;
DXGIManager g_DXGIManager;
RECT rcDim;

void DXGIScreepCap() {
	g_DXGIManager.GetOutputRect(rcDim);
	DWORD dwWidth = rcDim.right - rcDim.left;
	DWORD dwHeight = rcDim.bottom - rcDim.top;
	DWORD dwBufSize = dwWidth * dwHeight * 4;
	if (configsManager.screenSizeChanged) {
		if (ScreenData) {
			free(ScreenData);
		}
		ScreenData = (BYTE*)malloc(dwBufSize);
		configsManager.screenSizeChanged = false;
	}

	HRESULT hr;

	int i = 0;
	do{
		hr = g_DXGIManager.GetOutputBits(ScreenData, rcDim);
		i++;
	} while (hr == DXGI_ERROR_WAIT_TIMEOUT && i < 2);
	/*
	if (FAILED(hr) || hr == DXGI_ERROR_WAIT_TIMEOUT){
		ScreenData = NULL;
	}*/
}



void setScreenSize() {
	configsManager.screenSizeChanged = true;
	if (configsManager.multiMonitorSupport.load()) {
		configsManager.ScreenX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		configsManager.ScreenY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}
	else {
		configsManager.ScreenX = GetSystemMetrics(SM_CXSCREEN);
		configsManager.ScreenY = GetSystemMetrics(SM_CYSCREEN);
	}
}

void GDIScreenCap(){
	HDC hScreen = GetDC(GetDesktopWindow());
	HDC hdcMem = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, configsManager.ScreenX, configsManager.ScreenY);
	HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, configsManager.ScreenX, configsManager.ScreenY, hScreen, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = configsManager.ScreenX;
	bmi.biHeight = -configsManager.ScreenY;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

	if (configsManager.screenSizeChanged) {
		if (ScreenData)
			free(ScreenData);
		ScreenData = (BYTE*)malloc(4 * configsManager.ScreenX * configsManager.ScreenY);
		configsManager.screenSizeChanged = false;
	}

	GetDIBits(hdcMem, hBitmap, 0, configsManager.ScreenY, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	ReleaseDC(GetDesktopWindow(), hScreen);
	DeleteDC(hdcMem);
	DeleteObject(hBitmap);
}



inline int PosB(int x, int y)
{
	return ScreenData[4 * ((y*configsManager.ScreenX) + x)];
}

inline int PosG(int x, int y)
{
	return ScreenData[4 * ((y*configsManager.ScreenX) + x) + 1];
}

inline int PosR(int x, int y)
{
	return ScreenData[4 * ((y*configsManager.ScreenX) + x) + 2];
}

RGB getPixelAvg(unsigned int xStart, unsigned int yStart, unsigned int xEnd, unsigned int yEnd){
	unsigned long sumRed = 0;
	unsigned long sumGreen = 0;
	unsigned long sumBlue = 0;
	unsigned long totalZonePixels = 0;
	unsigned long numOfSkippedPixels = 0;
	const short lowColorDiff = 25;

	for (unsigned int x = xStart; x < xEnd; x = x + configsManager.stepX) {
		for (unsigned int y = yStart; y < yEnd; y = y + configsManager.stepY) {
			totalZonePixels++;
			int pixelRed = PosR(x, y);
			int pixelGreen = PosG(x, y);
			int pixelBlue= PosB(x, y);
			if (configsManager.filterBadColors) {
				short rgbSum = pixelRed + pixelGreen + pixelBlue;
				if (rgbSum < 50 || rgbSum > 762) { //254*3 = 762
					numOfSkippedPixels++;
					continue; //ignore extremely dark/bright colors as they just ruin the average
				}
				else if (abs(pixelRed - pixelGreen) <= lowColorDiff && abs(pixelRed - pixelBlue) <= lowColorDiff && abs(pixelGreen - pixelBlue) <= lowColorDiff) {
					numOfSkippedPixels++;
					continue; //I noticed that when the difference between R, G and B is less than a small number (say 25) then the color is "dark", thus it's better to skip it
				}
			}
			sumRed = sumRed + pixelRed;
			sumGreen = sumGreen + pixelGreen;
			sumBlue = sumBlue + pixelBlue;
		}
	}
	
	unsigned long totalFilteredPixels;
	if (numOfSkippedPixels >= totalZonePixels) {
		totalFilteredPixels = totalZonePixels; //to avoid cases such as division by zero or by a negative number, we should just ignore the skipped pixels
	}
	else {
		totalFilteredPixels = totalZonePixels - numOfSkippedPixels;
	}

	RGB avgRgb;
	avgRgb.red = sumRed / totalFilteredPixels;
	avgRgb.green = sumGreen / totalFilteredPixels;
	avgRgb.blue = sumBlue / totalFilteredPixels;
	
	return avgRgb;
}



const char* toString(CorsairError error){
	switch (error) {
		case CE_Success:
			return "CE_Success";
		case CE_ServerNotFound:
			return "CE_ServerNotFound";
		case CE_NoControl:
			return "CE_NoControl";
		case CE_ProtocolHandshakeMissing:
			return "CE_ProtocolHandshakeMissing";
		case CE_IncompatibleProtocol:
			return "CE_IncompatibleProtocol";
		case CE_InvalidArguments:
			return "CE_InvalidArguments";
		default:
			return "unknown error";
	}
}

void mapDeviceToSortedLeds(short deviceIndex, CorsairDeviceInfo *deviceInfo) {
	std::set<CorsairLedPosition, postitionComparator> sortedLedPositionsSet;
	const auto ledPositions = CorsairGetLedPositionsByDeviceIndex(deviceIndex);
	if (ledPositions) {
		for (auto i = 0; i < ledPositions->numberOfLed; i++) {
			const auto ledPos = ledPositions->pLedPosition[i];
			sortedLedPositionsSet.insert(ledPos);
		}
		deviceToLedPositionsMap[deviceInfo->type] = sortedLedPositionsSet;
	}
}


std::vector<CorsairLedColor> getAvailableKeys(){
	auto colorsSet = std::unordered_set<CorsairLedId>();
	for (short deviceIndex = 0, size = CorsairGetDeviceCount(); deviceIndex < size; deviceIndex++) {
		if (const auto deviceInfo = CorsairGetDeviceInfo(deviceIndex)) {
			switch (deviceInfo->type) {
			case CDT_Mouse: {
				auto numberOfKeys = deviceInfo->physicalLayout - CPL_Zones1 + 1;
				for (auto i = 0; i < numberOfKeys; i++) {
					const auto ledId = static_cast<CorsairLedId>(CLM_1 + i);
					colorsSet.insert(ledId);
				}
				colorsSet.insert(static_cast<CorsairLedId>(CLM_5));
				colorsSet.insert(static_cast<CorsairLedId>(CLM_6));
			} break;
			case CDT_Headset: {
				colorsSet.insert(CLH_LeftLogo);
				colorsSet.insert(CLH_RightLogo);
			} break;
			case CDT_Keyboard: {
				if (configsManager.keyboardZoneColoring) {
					mapDeviceToSortedLeds(deviceIndex, deviceInfo);
					break;
				}
			}
			case CDT_MouseMat: {
				if (configsManager.mousePadZoneColoring && deviceInfo->type != CDT_Keyboard) {
					mapDeviceToSortedLeds(deviceIndex, deviceInfo);
					break;
				}
			}
			case CDT_HeadsetStand:
			case CDT_CommanderPro:
			case CDT_LightingNodePro:
			case CDT_MemoryModule:
			case CDT_Cooler: {
				const auto ledPositions = CorsairGetLedPositionsByDeviceIndex(deviceIndex);
				if (ledPositions) {
					for (auto i = 0; i < ledPositions->numberOfLed; i++) {
						const auto ledId = ledPositions->pLedPosition[i].ledId;
						colorsSet.insert(ledId);
					}
				}
			} break;
			default:
				break;
			}
		}
	}

	std::vector<CorsairLedColor> colorsVector;
	colorsVector.reserve(colorsSet.size());
	for (const auto &ledId : colorsSet) {
		colorsVector.push_back({ ledId, 0, 0, 0 });
	}
	return colorsVector;
}


std::set<int> resizeSet(std::set<int> someSet, int desiredSize) {
	std::set<int>::iterator it = someSet.begin();
	const int setSize = someSet.size();
	int counter = 0;
	while (it != someSet.end()) {
		std::set<int>::iterator current = it++;
		double div = static_cast<double>(setSize) / desiredSize;
		if (counter % (static_cast<int>(ceil(div))) != 0) {
			someSet.erase(current);
		}
		counter++;
	}
	return someSet;
}


void avgColorByZone(int horizontalZones, int verticalZones) {
	for (int horizontalZoneId = 0; horizontalZoneId < horizontalZones; horizontalZoneId++) {
		for (int verticalZoneId = 0; verticalZoneId < verticalZones; verticalZoneId++) {
			unsigned int xStart = horizontalZoneId * (configsManager.ScreenX / horizontalZones);
			unsigned int yStart = verticalZoneId * (configsManager.ScreenY / verticalZones);
			unsigned int xEnd = (horizontalZoneId + 1) * (configsManager.ScreenX / horizontalZones);
			unsigned int yEnd = (verticalZoneId + 1) * (configsManager.ScreenY / verticalZones);
			RGB avgRgbPerZone = getPixelAvg(xStart, yStart, xEnd, yEnd);
			Zone zone = {horizontalZoneId, verticalZoneId};
			zoneToRGBmap[zone] = { avgRgbPerZone.red, avgRgbPerZone.green, avgRgbPerZone.blue};
		}
	}
}


void mapLedsToZones() {
	for (auto& device : deviceToLedPositionsMap) {
		auto currentDevice = device.first;
		switch (currentDevice) {
		case CDT_MouseMat:
		case CDT_Keyboard: {
			std::set<int> leftValuesSet;
			std::set<int> topValuesSet;
			for (auto& ledPos : deviceToLedPositionsMap[currentDevice]) {
				if (leftValuesSet.find(ledPos.left) == leftValuesSet.end()) {
					leftValuesSet.insert(ledPos.left);
				}
				if (topValuesSet.find(ledPos.top) == topValuesSet.end()) {
					topValuesSet.insert(ledPos.top);
				}
			}


			leftValuesSet = resizeSet(leftValuesSet, configsManager.horizontalZones);
			topValuesSet = resizeSet(topValuesSet, configsManager.verticalZones);

			for (auto& ledPos : deviceToLedPositionsMap[currentDevice]) {
				std::set<int>::iterator leftIterator = leftValuesSet.begin();
				int counterX = 0;
				while (leftIterator != leftValuesSet.end()) {
					std::set<int>::iterator current = leftIterator++;
					if (*current >= ledPos.left) {
						break;
					}
					counterX++;
				}

				std::set<int>::iterator topIterator = topValuesSet.begin();
				int counterY = 0;
				while (topIterator != topValuesSet.end()) {
					std::set<int>::iterator current = topIterator++;
					if (*current >= ledPos.top) {
						break;
					}
					counterY++;
				}
				ledPosToZoneMap[ledPos] = { counterX , counterY };
			}
			deviceLedsToZonesMap[currentDevice] = ledPosToZoneMap;
		} break;
		default:
			break;
		}
	}
}


void zoneColoring() {
	avgColorByZone(configsManager.horizontalZones, configsManager.verticalZones);
	for (auto& device : deviceLedsToZonesMap) {
		auto currentDevice = device.first;
		switch (currentDevice) {
		case CDT_MouseMat:
		case CDT_Keyboard: {
			std::map<CorsairLedPosition, Zone, postitionComparator> ledsZonesMap = deviceLedsToZonesMap[currentDevice];
			std::vector<CorsairLedColor> colorsVector;
			colorsVector.reserve(ledsZonesMap.size());
			for (auto& ledPosIter : ledsZonesMap) {
				CorsairLedPosition ledPos = ledPosIter.first;
				Zone currentZone = ledsZonesMap[ledPos];
				RGB avgZoneRGB = zoneToRGBmap[currentZone];
				colorsVector.push_back({ ledPos.ledId, avgZoneRGB.red, avgZoneRGB.green, avgZoneRGB.blue });
			}
			CorsairSetLedsColorsAsync(static_cast<int>(colorsVector.size()), colorsVector.data(), nullptr, nullptr);
		} break;
		default:
			break;
		}
	}
}


void setLedsColor(RGB avgRgb, std::vector<CorsairLedColor> &ledColorsVec) {
	for (auto &ledColor : ledColorsVec) {
		ledColor.r = avgRgb.red;
		ledColor.g = avgRgb.green;
		ledColor.b = avgRgb.blue;
	}
	CorsairSetLedsColorsAsync(static_cast<int>(ledColorsVec.size()), ledColorsVec.data(), nullptr, nullptr);
}


BOOL ctrl_handler(DWORD event){
	if (event == CTRL_CLOSE_EVENT) { //Handling window close event
		configsManager.continueExecution = false;
		return TRUE;
	}
	return FALSE;
}


void initDXGI() {
	CoInitialize(NULL);
	//g_DXGIManager.SetCaptureSource(CSDesktop);
	g_DXGIManager.SetCaptureSourceByIndex(configsManager.pinToMonitor);
	if (g_DXGIManager.Init() != S_OK) {
		std::cout << "Failed to initialize DXGI. Using GDI instead.\n" << std::endl;
		configsManager.DXGI = false;
	}
}

void checkMonitorMode() {
	if (configsManager.multiMonitorSupport && configsManager.pinToMonitor != -1) {
		std::cout << "Warning: PinToMonitor is set to " << configsManager.pinToMonitor << " while Multi-monitor mode is on.\nIgnoring the PinToMonitor setting.\n" << std::endl;
		configsManager.pinToMonitor = -1;
	}
}


void printRefeshInterval() {
	std::cout << "Refresh interval set to " << configsManager.sleepDuration.load() << " milliseconds" << std::endl;
}

void printOptions() {
	std::string options = "";
	options = options + "\nOptions:" + "\n";
	options = options + "+ : increase the color refresh interval by 100ms (less CPU usage)" + "\n";
	options = options + "- : decrease the color refresh interval by 100ms (more CPU usage)" + "\n";
	options = options + "1 : single monitor mode (disable multi-monitor support - less CPU usage)" + "\n";
	options = options + "2 : multi-monitor mode" + "\n";
	options = options + "o : print the options" + "\n";
	options = options + "s : print the loaded settings" + "\n";
	options = options + "r : reload the settings file (works when files are extracted)" + "\n";
	options = options + "a : switch API (choices: DXGI, DGI)" + "\n";
	options = options + "q : quit the app\n" + "\n";
	std::cout << options << std::endl;
}

void handleOptions() {
	printOptions();
	while (configsManager.continueExecution) {
		char c = getchar();
		switch (c) {
		case '-':
			if (configsManager.sleepDuration.load() > 100) {
				configsManager.sleepDuration -= 100;
				printRefeshInterval();
			}
			else {
				std::cout << "Cannot decrease the refesh interval any further" << std::endl;
			}
			break;
		case '+':
			if (configsManager.sleepDuration.load() < 1000) {
				configsManager.sleepDuration += 100;
				printRefeshInterval();
			}
			else {
				std::cout << "Cannot increase the refesh interval any further" << std::endl;
			}
			break;
		case '1':
			configsManager.multiMonitorSupport = false;
			if (configsManager.pinToMonitor == -1 || configsManager.pinToMonitor == 0) {
				configsManager.pinToMonitor = 1;
			}
			std::cout << "Multi-monitor support is OFF\n" << std::endl;
			setScreenSize();
			if (configsManager.DXGI) {
				initDXGI();
			}
			break;
		case '2':
			configsManager.multiMonitorSupport = true;
			if (configsManager.pinToMonitor != -1) {
				configsManager.pinToMonitor = -1;
			}
			std::cout << "Multi-monitor support is ON\n" << std::endl;
			setScreenSize();
			if (configsManager.DXGI) {
				initDXGI();
			}
			break;
		case 'o':
		case 'O':
			printOptions();
			break;
		case 's':
		case 'S':
			configsManager.printLoadedSettings();
			break;
		case 'Q':
		case 'q':
			configsManager.continueExecution = false;
			break;
		case 'r':
		case 'R':
			configsManager.loadConfigsFromSettingsFile();
			checkMonitorMode();
			setScreenSize();
			configsManager.printLoadedSettings();
			if (configsManager.DXGI) {
				initDXGI();
			}
			break;
		case 'a':
		case 'A':
			configsManager.DXGI = !configsManager.DXGI;
			std::cout << "Using " << (configsManager.DXGI ? "DXGI" : "GDI") << " API to capture the screen." << std::endl;
			if (configsManager.DXGI) {
				initDXGI();
			}
			else {
				std::cout << "Please note that DXGI is much faster than GDI." << std::endl;
			}
			break;
		default:
			break;
		}
	}
}

int main(){
	std::cout << "Starting app" << std::endl;
	std::cout << "App version: " << version << "\n" << std::endl;
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(ctrl_handler), TRUE);

	CorsairPerformProtocolHandshake();
	if (const auto error = CorsairGetLastError()) {
		std::cout << "Handshake failed: " << toString(error) << "\n" << std::endl;
		std::cout << "Please make sure that either iCUE or CUE are running before you start the app." << std::endl;
		std::cout << "Also, make sure that \"Enable SDK\" is ticked in the settings." << std::endl;
		std::cout << "If it's already on, try to set it to off then back on again.\n" << std::endl;
		std::cout << "Enter any key to quit..." << std::endl;
		getchar();
		return -1;
	}
	CorsairRequestControl(CAM_ExclusiveLightingControl);
	//CorsairSetLayerPriority(129);

	configsManager.loadConfigsFromSettingsFile(); //load configs from settings.ini
	checkMonitorMode();
	setScreenSize();
	
	std::thread updateThread([] {
		if (configsManager.checkForUpdateFF) {
			std::cout << "Checking for updates..." << std::endl;
			UpdateManager::checkForAnUpdate();
		}
	});

	if (configsManager.DXGI) {
		initDXGI();
	}

	


	auto colorsVector = getAvailableKeys();
	std::cout << "Available LED keys: " << colorsVector.size() << std::endl;
	if (colorsVector.empty() && !configsManager.keyboardZoneColoring && !configsManager.mousePadZoneColoring) {
		std::cout << "No LED keys available" << "\nPress any key to quit." << std::endl;
		getchar();
		return 1;
	}

	std::cout << "Running..." << "\n" << std::endl;
	mapLedsToZones();
	std::thread lightingThread([&colorsVector] {
		while (configsManager.continueExecution) {
			if (configsManager.DXGI) {
				DXGIScreepCap();
				if (ScreenData == NULL) {
					continue;
				}
			}
			else {
				GDIScreenCap();
			}
			setLedsColor(getPixelAvg(0, 0, configsManager.ScreenX, configsManager.ScreenY), colorsVector);
			if (configsManager.keyboardZoneColoring || configsManager.mousePadZoneColoring) {
				zoneColoring();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(configsManager.sleepDuration.load()));
		}
	});
	
	handleOptions();

	lightingThread.join();
	updateThread.join();

	return 0;
}


