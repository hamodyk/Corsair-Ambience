# Corsair Ambience
A very lightweight app that allows you to set your devices' led colors based on what you see on your screen.


# Demo
https://www.youtube.com/watch?v=Qqj28Wc86cg

Keep in mind that this was filmed using my phone, so I can assure you it looks much better in real life.

# Download & Run

[Click here to download the latest version (v1.3)](https://github.com/hamodyk/Corsair-Ambience/releases/download/v1.3/Corsair.Ambience.v1.3.rar)

#### Please make sure that you have either iCUE or CUE running before you start the app.

To run it, simply open the .rar file, inside it you'll find a folder called "Release", open it and double click on "Corsair Ambience.exe".

You can also extract the .rar file, which will create the Release folder with the .exe inside it.

# Project installation
To install and debug the project you need to do the following:

1. Start a C++ console project in Visual Studio 2017
2. Copy include, lib and redist folder into the Root of the project
3. Go to Project properties-->C/C++-->General-->Additional Include Directories, Add the follow line: $(SolutionDir)include\
4. Go to Project properties-->Linker->General-->Additional Library Directories, Add the follow line: $(SolutionDir)lib\x64\
5. Go to Project properties-->Linker->Input-->Additional Dependencies, add follow line in front of it: libcurl.lib;CUESDK.x64_2013.lib;
6. Go to Project properties-->Build Events-->Pre-Build Event-->Command Line, add following line: xcopy  /Y /I  "$(SolutionDir)settings.ini" "$(OutDir)"
6. Go to Project properties-->Build Events-->Post-Build Event-->Command Line, add following line: xcopy /Y /I "$(SolutionDir)redist\x64\*" "$(OutDir)"
7. Apply all of those

Notes: 
- All of the above should already be set for you if you just clone the project
- You might need to retarget the solution (Right click on the Solution 'Corsair Ambience' then choose 'Retarget solution')
- Your run mode (Debug or Release) must be set to x64 for the settings above to work


# DISCLAIMERS: 
- USE AT YOUR OWN RISK, I'M NOT RESPONSIBLE FOR ANY DAMAGES YOU CAUSE TO THE SYSTEM/HARDWARE THAT YOU RUN/INSTALL THIS APPLICATION ON.

- I'M NOT ASSOCIATED/AFFILIATED WITH CORSAIR IN ANY SORT OF WAY. I'M JUST USING THEIR SDK.


# Compatibility
- Windows (64 bit)
- Every Corsair RGB device (as of 29/12/2018)

# Road Map
- Create a GUI
- Add more configurations (such as sleep time, number of pixels to sample etc...)
- Add more coloring options (such as most dominant color, elimination of dark colors etc...)
- Make it an installable app
- Add option to start the app on Windows start up
- Multi-monitor support - *done*
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

- https://github.com/curl/curl - for getting the latest version of the app from Github
- https://github.com/nlohmann/json - for parsing curl's http response as json and extracting the latest version
- https://github.com/brofield/simpleini - for the parsing the settings file

# Donations
If you enjoy this application and would like to support its development: 

- Paypal: https://www.paypal.me/cytrixghost
- Steam: https://steamcommunity.com/tradeoffer/new/?partner=63409948&token=qKZ6lsHT 

Thanks :)
