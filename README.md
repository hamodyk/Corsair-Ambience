# Corsair Ambience
![](https://img.shields.io/github/downloads/hamodyk/Corsair-Ambience/total.svg)
![](https://img.shields.io/github/license/hamodyk/Corsair-Ambience.svg)

A very lightweight app that allows you to set your devices' led colors based on what you see on your screen.


# Demo
https://www.youtube.com/watch?v=Qqj28Wc86cg

Keep in mind that this was filmed using my phone, so I can assure you it looks much better in real life.

# Download & Run

To download the latest version, click on one of the following links:

[v2.1 - Installer (recommended)](https://github.com/hamodyk/Corsair-Ambience/releases/download/v2.1/AmbienceInstaller.msi)

[v2.1 - Portable rar](https://github.com/hamodyk/Corsair-Ambience/releases/download/v2.1/Corsair.Ambience.v2.1.rar)



#### Please make sure that you have either iCUE or CUE running before you start the app.

To run it, simply open the .rar file, inside it you'll find a folder called "Corsair Ambience v...", open it and double click on "Corsair Ambience.exe".
You can also extract the .rar file, which will create a folder with the .exe inside it.

To install the project on your Visual Studio, [follow the steps here](https://github.com/hamodyk/Corsair-Ambience/blob/master/Project%20installation)


# DISCLAIMERS: 
- USE AT YOUR OWN RISK, I'M NOT RESPONSIBLE FOR ANY DAMAGES YOU CAUSE TO THE SYSTEM/HARDWARE THAT YOU RUN/INSTALL THIS APPLICATION ON.

- I'M NOT ASSOCIATED/AFFILIATED WITH CORSAIR IN ANY SORT OF WAY. I'M JUST USING THEIR SDK.


# Compatibility
- Windows (64 bit)
- Every Corsair RGB device (as of 29/12/2018)

# Road Map
- Create a GUI
- Add more configurations (such as sleep time, number of pixels to sample etc...) - ![](https://img.shields.io/badge/status-done-brightgreen.svg)
- Add more coloring options (such as most dominant color, elimination of dark colors etc...)
- Make it an installable app
- Add option to start the app on Windows start up
- Multi-monitor support - ![](https://img.shields.io/badge/status-done-brightgreen.svg)
- Improve performance
- Performance impact testing

# Q&A
Q: Does the app also work while using fullscreen applications such as games?

A: Yes, but apparently it must be fullscreen windowed or borderless (not fullscreen exclusive). Tested with PUBG, CSGO and Battlefield V.

Q: Will it hinder my computer's performance?

A: No. The app is very lightweight as it uses only ~ 1% - 2.5% CPU (tested on my i5 3570k) and 15 MB of RAM. You can also configure it to make it even less CPU demanding.

Q: Does it support multi-monitor setups?

A: Yes

# Dependencies

- [Libcurl](https://github.com/curl/curl) - for using http requests get the latest version of the app from Github
- [JSON for modern C++](https://github.com/nlohmann/json) - for parsing curl's http response as json and extracting the latest version
- [SimpleINI](https://github.com/brofield/simpleini) - for the parsing the settings file
- [DXGI Capture Sample](https://github.com/pgurenko/DXGICaptureSample) - for using DXGI API instead of GDI to capture the screen

# Donations
If you enjoy this application and would like to support its development, here's my: 

- Paypal: https://www.paypal.me/cytrixghost
- Steam: https://steamcommunity.com/tradeoffer/new/?partner=63409948&token=qKZ6lsHT 

Thanks :)
