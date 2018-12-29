#ifdef __APPLE__
#include <CUESDK/CUESDK.h>
#else
#include <CUESDK.h>
#endif


#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <windows.h>
#include <cstdlib>
#include <unordered_set>
#include <SimpleIni.h>
#include <nlohmann/json.hpp>

#define version "v1.3"
#define settingsFileName "settings.ini"

using json = nlohmann::json;

struct RGB { int red; int green; int blue; };
int ScreenX = 0;
int ScreenY = 0;
int stepX = 5;
int stepY = 5;
BYTE* ScreenData = 0;
CSimpleIniA ini;


std::atomic_bool continueExecution{ true };
std::atomic_bool multiMonitorSupport{ true };
std::atomic_int sleepDuration{ 500 };
bool checkForUpdateFF = true;

void ScreenCap()
{
	HDC hScreen = GetDC(GetDesktopWindow());
	if (multiMonitorSupport.load()) {
		ScreenX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		ScreenY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}
	else {
		ScreenX = GetDeviceCaps(hScreen, HORZRES);
		ScreenY = GetDeviceCaps(hScreen, VERTRES);
	}

	HDC hdcMem = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, ScreenX, ScreenY);
	HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, ScreenX, ScreenY, hScreen, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = ScreenX;
	bmi.biHeight = -ScreenY;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

	if (ScreenData)
		free(ScreenData);
	ScreenData = (BYTE*)malloc(4 * ScreenX * ScreenY);

	GetDIBits(hdcMem, hBitmap, 0, ScreenY, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	ReleaseDC(GetDesktopWindow(), hScreen);
	DeleteDC(hdcMem);
	DeleteObject(hBitmap);
}



inline int PosB(int x, int y)
{
	return ScreenData[4 * ((y*ScreenX) + x)];
}

inline int PosG(int x, int y)
{
	return ScreenData[4 * ((y*ScreenX) + x) + 1];
}

inline int PosR(int x, int y)
{
	return ScreenData[4 * ((y*ScreenX) + x) + 2];
}

RGB getPixelAvg()
{
	int sumRed = 0;
	int sumGreen = 0;
	int sumBlue = 0;

	int numOfSkippedPixels = 0;
	const int lowColorDiff = 25;

	for (int x = 0; x < ScreenX; x = x + stepX) {
		for (int y = 0; y < ScreenY; y = y + stepY) {
			int pixelRed = PosR(x, y);
			int pixelGreen = PosG(x, y);
			int pixelBlue= PosB(x, y);
			if (pixelRed + pixelGreen + pixelBlue < 50 || pixelRed + pixelGreen + pixelBlue > 254*3) { 
				numOfSkippedPixels++;
				continue; //ignore extremely dark/bright colors as they just ruin the average
			}
			else if (abs(pixelRed - pixelGreen) <= lowColorDiff && abs(pixelRed - pixelBlue) <= lowColorDiff && abs(pixelGreen - pixelBlue) <= lowColorDiff) {
				numOfSkippedPixels++;
				continue; //I noticed that when the difference between R, G and B is less than a small number (say 25) then the color is "dark", thus it's better to skip it
			}
			sumRed = sumRed + pixelRed;
			sumGreen = sumGreen + pixelGreen;
			sumBlue = sumBlue + pixelBlue;
		}
	}
	
	const int totalScreenPixels = (ScreenX / stepX) * (ScreenY / stepY);
	int totalFilteredPixels;
	if (numOfSkippedPixels >= totalScreenPixels) {
		totalFilteredPixels = totalScreenPixels; //to avoid cases such as division by zero or by a negative number, we should just ignore the skipped pixels
	}
	else {
		totalFilteredPixels = totalScreenPixels - numOfSkippedPixels;
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

std::vector<CorsairLedColor> getAvailableKeys(){
	auto colorsSet = std::unordered_set<CorsairLedId>();
	for (int deviceIndex = 0, size = CorsairGetDeviceCount(); deviceIndex < size; deviceIndex++) {
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
			case CDT_Keyboard:
			case CDT_MouseMat:
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
		continueExecution = false;
		return TRUE;
	}
	return FALSE;
}

void printRefeshInterval() {
	std::cout << "Refresh interval set to " << sleepDuration.load() << " milliseconds" << std::endl;
}

void printOptions() {
	std::string options = "";
	options = options + "\nOptions:" + "\n";
	options = options + "+ : increase the color refresh interval by 100ms (less CPU usage)" + "\n";
	options = options + "- : decrease the color refresh interval by 100ms (more CPU usage)" + "\n";
	options = options + "1 : single monitor mode (disable multi-monitor support - less CPU usage)" + "\n";
	options = options + "2 : multi-monitor mode" + "\n";
	options = options + "o : print the options" + "\n";
	options = options + "q : quit the app\n" + "\n";
	std::cout << options << std::endl;
}

bool stringToBool(std::string str) {
	if (str.compare("true") == 0 || str.compare("on") == 0 || str.compare("1") == 0) {
		return true;
	}
	else {
		return false;
	}
}

void printLoadedSettings() {
	std::cout << "Refresh interval: " << sleepDuration.load() << " milliseconds" << std::endl;
	std::cout << "Horizontal Step: " << stepX << std::endl;
	std::cout << "Vertical Step: " << stepY << std::endl;
	std::cout << "Check for an update on start up is " << (checkForUpdateFF ? "ON" : "OFF") << std::endl;
	std::cout << "Multi-monitor mode is " << (multiMonitorSupport ? "ON" : "OFF") << "\n" << std::endl;
}


int getConfigValAsInt(const char* section, const char* key, int defaultVal, int rangeLowerBound, int rangeUpperBound) {
	int res;
	try {
		res = std::stoi(ini.GetValue(section, key), nullptr);
		if (rangeLowerBound && rangeUpperBound && (res < rangeLowerBound || res > rangeUpperBound)) {
			std::cout << "Error: the setting \"" <<  key << "\" must be between " << rangeLowerBound << " and " << rangeUpperBound << std::endl;
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

void handleFileConfigurations() {
	ini.SetUnicode();
	int rc = ini.LoadFile(settingsFileName);
	if (rc < 0) {
		std::cout << "Failed to load the settings file: " << settingsFileName << std::endl;
		return;
	}

	sleepDuration = getConfigValAsInt("CONFIGS", "refreshInterval", sleepDuration, 15, 1000);

	stepX = getConfigValAsInt("CONFIGS", "horizontalStep", stepX, 1, 32);
	stepY = getConfigValAsInt("CONFIGS", "verticalStep", stepY, 1, 32);

	multiMonitorSupport = stringToBool(ini.GetValue("CONFIGS", "multiMonitorSupport"));
	checkForUpdateFF = stringToBool(ini.GetValue("CONFIGS", "checkForUpdateOnStartup"));
}

void handleConfigurations() {

	printLoadedSettings();

	printOptions();

	while (continueExecution) {
		char c = getchar();
		switch (c) {
			case '-':
				if (sleepDuration.load() > 100) {
					sleepDuration -= 100;
					printRefeshInterval();
				}
				else {
					std::cout << "Cannot decrease the refesh interval any further" << std::endl;
				}
				break;
			case '+':
				if (sleepDuration.load() < 1000) {
					sleepDuration += 100;
					printRefeshInterval();
				}
				else {
					std::cout << "Cannot increase the refesh interval any further" << std::endl;
				}
				break;
			case '1':
				multiMonitorSupport = false;
				std::cout << "Multi-monitor support is OFF. Using only the primary monitor to calculate the average.\n" << std::endl;
				break;
			case '2':
				multiMonitorSupport = true;
				std::cout << "Multi-monitor support is ON. Using all monitors to calculate the average.\n" << std::endl;
				break;
			case 'o':
			case 'O':
				printOptions();
				break;
			case 'Q':
			case 'q':
				continueExecution = false;
				break;
			default:
				break;
			}
	}
}


size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s){
	size_t newLength = size * nmemb;
	size_t oldLength = s->size();
	try{
		s->resize(oldLength + newLength);
	}
	catch (std::bad_alloc &e){
		//handle memory problem
		std::cout << "Error: couldn't allocate enough memory for the HTTP response" << std::endl;
		std::cout << e.what() << std::endl;
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
	return size * nmemb;
}

void checkForAnUpdate() {
	try {
		CURL *curl;
		CURLcode res;
		std::string response;
		curl_global_init(CURL_GLOBAL_DEFAULT);
		struct curl_slist *list = NULL;
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/hamodyk/Corsair-Ambience/releases/latest");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
			list = curl_slist_append(list, "User-Agent: C/1.0");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

			#ifdef SKIP_PEER_VERIFICATION
			/*
			* If you want to connect to a site who isn't using a certificate that is
			* signed by one of the certs in the CA bundle you have, you can skip the
			* verification of the server's certificate. This makes the connection
			* A LOT LESS SECURE.
			*
			* If you have a CA cert for the server stored someplace else than in the
			* default bundle, then the CURLOPT_CAPATH option might come handy for
			* you.
			*/
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			#endif

			#ifdef SKIP_HOSTNAME_VERIFICATION
			/*
			* If the site you're connecting to uses a different host name that what
			* they have mentioned in their server certificate's commonName (or
			* subjectAltName) fields, libcurl will refuse to connect. You can skip
			* this check, but this will make the connection less secure.
			*/
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			#endif

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if (res != CURLE_OK) {
				//fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
				std::cout << "Error: Unable to retrieve information about the latest version from Github" << std::endl;
				std::cout << curl_easy_strerror(res) << "\n" << std::endl;
				return;
			}

			/* always cleanup */
			curl_easy_cleanup(curl);
			curl_slist_free_all(list);
		}

		curl_global_cleanup();

		auto js = json::parse(response);
		std::string latestVersion = js["tag_name"];
		std::string currentVersion = version;
		if (latestVersion.compare(currentVersion) != 0) { //if they are not equal
			std::cout << "a new version (" << latestVersion << ") is available to download!" << std::endl;
			std::cout << "Visit " << "https://github.com/hamodyk/Corsair-Ambience" << " for more info" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cout << "Error: Unable to retrieve information about the latest version from Github" << std::endl;
		std::cout << e.what() << "\n" << std::endl;
	}
}


int main()
{
	std::cout << "Starting app" << std::endl;
	std::cout << "App version: " << version << "\n" << std::endl;
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(ctrl_handler), TRUE);
	handleFileConfigurations(); //load configs from settings.ini
	std::thread updateThread([] {
		if (checkForUpdateFF) {
			std::cout << "Checking for updates..." << std::endl;
			checkForAnUpdate();
		}
	});
		
	
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

	auto colorsVector = getAvailableKeys();
	std::cout << "Available LED keys: " << colorsVector.size() << std::endl;
	if (colorsVector.empty()) {
		std::cout << "No LED keys available" << "\nPress any key to quit." << std::endl;
		getchar();
		return 1;
	}

	std::cout << "Running..." << "\n" << std::endl;
	
	std::thread lightingThread([&colorsVector] {
		while (continueExecution) {
			ScreenCap();
			setLedsColor(getPixelAvg(), colorsVector);
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration.load()));
		}
	});
	
	handleConfigurations();

	lightingThread.join();
	updateThread.join();

	return 0;
}


