# Corsair-Ambience
Corsair RGB Ambience - Set your Corsair devices' led colors based on what you see in your screen


# Demo
https://www.youtube.com/watch?v=Qqj28Wc86cg

Keep in mind that this was filmed using my phone, so I can assure you it looks much better in real life.

# Download & Run
Download the app from here: https://drive.google.com/open?id=1o6pSWMzm8f0APzhX-hu9fbXTHlgnO2pN

To run it, simply open the .rar file and double click on "Corsair Ambience.exe"

# Project installation
To install and debug the project you need to do the following:

1. Start a C++ console project in Visual Studio 2017
2. Copy include, lib and redist folder into the Root of the project
3. Go to Project properties-->C/C++-->General-->Additional Include Directories, Add the follow line $(SolutionDir)include\
4. Go to Project properties-->Linker->General-->Additional Library Directories, Add the follow line, $(SolutionDir)lib\x64\
5. Go to Project properties-->Linker->Input-->Additional Dependencies, add follow line in front of it, CUESDK.x64_2013.lib;
6. Go to Project properties-->Build Events-->Post-Build Event-->Command Line, add following line, xcopy /Y /I "$(SolutionDir)redist\x64\*" "$(OutDir)"
7. Apply all of those

Note: Your run mode (Debug or Release) must be set to x64 for the settings above to work


# DISCLAIMERS: 
- USE AT YOUR OWN RISK, I'M NOT RESPONSIBLE FOR ANY DAMAGES YOU CAUSE TO THE SYSTEM/HARDWARE THAT YOU RUN/INSTALL THIS APPLICATION ON.

- I'M NOT ASSOCIATED/AFFILIATED WITH CORSAIR IN ANY SORT OF WAY. I'M JUST USING THEIR SDK.


# Compatible Devices
Every Corsair RGB device should be compatible with this software except:
- RAM
- Liquid cooling solutions

Reason: Lack of SDK on Corsair's side

# Road Map
- Create a GUI
- Add more configurations (such as sleep time, number of pixels to sample etc...)
- Add more coloring options (such as most dominant color, elimination of dark colors etc...)
- Make it an installable app
- Add option to start the app on Windows start up
- Improve performance
- Performance impact testing

# Q&A
Q- Does the app also work while using fullscreen applications such as games?

A- Yes

Q- Will it hinder my computer's performance?

A- The app uses ~ 1% - 2.5% CPU and 9 MB of RAM, so I wouldn't think so. Some actual performance impact testing is required though.


# Troubleshooting
### Problem:
I'm getting the following error - "Handshake failed: CE_ServerNotFound"

### Solution: 
Make sure you have started iCUE or CUE, and make sure that the "Enable SDK" option is ticked in the settings. If it's already on, try to untick it and then tick it back.

# Donations
If you enjoy this application and would like to support its development: 

- Paypal: https://www.paypal.me/cytrixghost
- Steam: https://steamcommunity.com/tradeoffer/new/?partner=63409948&token=qKZ6lsHT 

Thanks :)
