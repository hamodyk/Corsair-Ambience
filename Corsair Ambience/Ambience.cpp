#ifdef __APPLE__
#include <CUESDK/CUESDK.h>
#else
#include <CUESDK.h>
#endif

#include <iostream>
#include <thread>
#include <vector>
#include<windows.h>



struct RGB { int red; int green; int blue; };

int ScreenX = 0;
int ScreenY = 0;
BYTE* ScreenData = 0;

void ScreenCap()
{
	HDC hScreen = GetDC(GetDesktopWindow());
	ScreenX = GetDeviceCaps(hScreen, HORZRES);
	ScreenY = GetDeviceCaps(hScreen, VERTRES);

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
	int stepX = 3;
	int stepY = 3;

	for (int x = 0; x < ScreenX; x = x + stepX) {
		for (int y = 0; y < ScreenY; y = y + stepY) {
			sumRed = sumRed + PosR(x, y);
			sumGreen = sumGreen + PosG(x, y);
			sumBlue = sumBlue + PosB(x, y);
		}
	}

	RGB avgRgb;
	avgRgb.red = sumRed / ((ScreenX / stepX) * (ScreenY / stepY));
	avgRgb.green = sumGreen / ((ScreenX / stepX) * (ScreenY / stepY));
	avgRgb.blue = sumBlue / ((ScreenX / stepX) * (ScreenY / stepY));

	return avgRgb;
}



const char* toString(CorsairError error)
{
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

std::vector<CorsairLedColor> getAvailableKeys()
{
	auto colorsVector = std::vector<CorsairLedColor>();
	for (auto deviceIndex = 0; deviceIndex < CorsairGetDeviceCount(); deviceIndex++) {
		if (auto deviceInfo = CorsairGetDeviceInfo(deviceIndex)) {
			switch (deviceInfo->type) {
			case CDT_Mouse: {
				auto numberOfKeys = deviceInfo->physicalLayout - CPL_Zones1 + 1;
				for (auto i = 0; i < numberOfKeys; i++) {
					auto ledId = static_cast<CorsairLedId>(CLM_1 + i);
					colorsVector.push_back(CorsairLedColor{ ledId, 0, 0, 0 });
				}
			} break;
			case CDT_Keyboard: {
				auto ledPositions = CorsairGetLedPositions();
				if (ledPositions) {
					for (auto i = 0; i < ledPositions->numberOfLed; i++) {
						auto ledId = ledPositions->pLedPosition[i].ledId;
						colorsVector.push_back(CorsairLedColor{ ledId, 0, 0, 0 });
					}
				}
			} break;
			case CDT_Headset: {
				colorsVector.push_back(CorsairLedColor{ CLH_LeftLogo, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLH_RightLogo, 0, 0, 0 });
			} break;
			case CDT_CommanderPro:
			case CDT_LightingNodePro: {
				auto ledPositions = CorsairGetLedPositionsByDeviceIndex(deviceIndex);
				if (ledPositions) {
					for (auto i = 0; i < ledPositions->numberOfLed; i++) {
						auto ledId = ledPositions->pLedPosition[i].ledId;
						colorsVector.push_back(CorsairLedColor{ ledId, 0, 0, 0 });
					}
				}
			} break;
			case CDT_MouseMat: {
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone1, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone2, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone3, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone4, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone5, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone6, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone7, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone8, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone9, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone10, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone11, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone12, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone13, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone14, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLMM_Zone15, 0, 0, 0 });
			} break;
			case CDT_HeadsetStand: {
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone1, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone2, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone3, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone4, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone5, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone6, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone7, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone8, 0, 0, 0 });
				colorsVector.push_back(CorsairLedColor{ CLHSS_Zone9, 0, 0, 0 });
			} break;
			default:
				break;
			}
		}
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

int main()
{
	CorsairPerformProtocolHandshake();
	if (const auto error = CorsairGetLastError()) {
		std::cout << "Handshake failed: " << toString(error) << "\nPress any key to quit." << std::endl;
		getchar();
		return -1;
	}

	auto colorsVector = getAvailableKeys();
	std::cout << "Available LED keys: " << colorsVector.size() << std::endl;
	if (colorsVector.empty()) {
		return 1;
	}

	std::thread lightingThread([&colorsVector] {
		while (1) {
			ScreenCap();
			setLedsColor(getPixelAvg(), colorsVector);
			//Sleep(500);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	});

	lightingThread.join();

	return 0;
}
