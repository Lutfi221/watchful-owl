# Watchful Owl

![Main menu of Watchful Owl](./docs/assets/main%20menu.png)

Watchful Owl is an app to track what apps that you use throughout the day. Every minute, Watchful Owl will log what apps are opened, and which of them you're currently using. It will also detect if you're away from the computer by checking the time of the last user input.

You can use the data logged to track how much time you actually spent working and procrastinating.

**[Download here](https://github.com/Lutfi221/watchful-owl/releases)**. Currently, only available on windows.

- [Watchful Owl](#watchful-owl)
  - [Instructions](#instructions)
    - [Download and install](#download-and-install)
    - [Setting Up Watchful Owl](#setting-up-watchful-owl)
    - [Make Watchful Owl Run Automatically](#make-watchful-owl-run-automatically)
  - [Settings](#settings)
  - [Logging Format](#logging-format)
  - [Inspiration](#inspiration)
  - [Notes for Developers](#notes-for-developers)
    - [Building](#building)
    - [Developing](#developing)

## Instructions

### Download and install

1. Go to [the release page](https://github.com/Lutfi221/watchful-owl/releases)
2. Find the file name that looks like `watchful-owl-win-{version}-x64.rar`, and click it to download.
3. Open the downloaded archive, and extract it to a folder. Any location will do.
4. You can now open `Watchful Owl.exe`

### Setting Up Watchful Owl

1. Open `Watchful Owl.exe`, and use your up and down arrow keys to move your selection.
2. If Watchful Owl is not running, select `Activate Watchful Owl` and press enter.
3. The program should now say `Watchful Owl is ACTIVE and currently logging your activity.`
4. Select `Exit` and press enter to exit.

### Make Watchful Owl Run Automatically

To make Watchful Owl run automatically and start logging when you turn on your computer,

1. Open `Watchful Owl.exe`.
2. Select `Configure Autorun` and press enter.
3. If autorun is not enabled, select `Enable Autorun` and press enter.
4. The program should now say `Autorun is ENABLED for the current user.`

In Windows, by enabling autorun, Watchful Owl will place a script called `Watchful Owl Perpetual.bat` in `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup`.

## Settings

Settings are stored in `config.json`. This is the default settings. When you change the settings, make sure to restart by deactivating and activating Watchful Owl.

```js
{
  "loggingInterval": 60, // How often to log (in seconds) opened apps
  "outDir": "./owl-logs", // Output directory for the log files
  "idleThreshold": 60 // How long to wait (in seconds) until assuming the user is away from the computer
}
```

## Logging Format

Every line in the log file are a separate and valid JSON object. Here is an example of the logging format.

```js
{
  "apps": [
    {
      "path": "C:\\Users\\Lutfi221\\AppData\\Local\\Programs\\Microsoft VS Code\\Code.exe",
      "title": "perpetual.cpp - watchful-owl - Visual Studio Code",
      "isActive": true // Indicates that the user is currently using this app.
    },
    { "path": "C:\\Windows\\explorer.exe", "title": "Watchful Owl" },
    {
      "path": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
      "title": "html - RegEx match open tags except XHTML self-contained tags - Stack Overflow"
    }
  ],
  "time": 1676257718 // The UNIX timestamp. The number of seconds since the unix epoch
}
```

## Inspiration

I got the inspiration to build Watchful Owl when I found out that Windows kept track of apps and files I've opened. It was surprising, when I pressed `Windows + TAB` and scrolled down, to see my past activity on display.

So, for privacy reasons, I turned the feature off. But, it made me realize how fascinating the data collected could be. If I have data of the apps that I use and how much time I spent on them for a long period of time, I could do some fancy data analysis and make some fancy visualizations.

So that's why I created Watchful Owl. Why the name 'Owl'? Well, it stands for opened windows logger.

## Notes for Developers

### Building

You will need [CMake](https://cmake.org/download/) in order to build Watchful Owl, and an internet connection for CMake to automatically download the project's dependencies: [FTXUI](https://github.com/ArthurSonzogni/FTXUI/) and [spdlog](https://github.com/gabime/spdlog).

```batch
git clone https://github.com/Lutfi221/watchful-owl.git
cd watchful-owl
mkdir build
```

The next command depends on the compiler you have installed in your system. If you have something other than MinGW, simply replace `MinGW Makefiles` with the correct generator name for your system. You can see a list of generators available by running `cmake --help`.

```batch
cmake -G "MinGW Makefiles" . -B build
```

If you want to change the build options, you can run `cmake-gui .`

And finally, to build Watchful Owl, run the following command.

```batch
cmake --build build
```

### Developing

When you are developing, remember to set `CMAKE_BUILD_TYPE` to `Debug`. This will append a `-DEBUG` suffix to `perpetual-owl.exe` and to its autorun script. So if an installed Watchful Owl is installed and running in the same system, the Watchful Owl being developed will not interfere with the installed Watchful Owl.
