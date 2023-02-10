#include <iostream>
#include <filesystem>
#include "ftxui/component/screen_interactive.hpp"

#include "constants.hpp"
#include "ui/ui.h"
#include "ui/browser.h"

using namespace std;

int main(int argc, char **argv)
{
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto config = loadConfig();
    auto mainPage = MainPage(&screen, &config);
    Browser browser = Browser(&screen, &mainPage);
    browser.load();
    return 0;
}
