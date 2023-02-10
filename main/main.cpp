#include <filesystem>
#include <iostream>

#include "ftxui/component/screen_interactive.hpp"

#include "constants.hpp"
#include "ui/browser.h"
#include "ui/ui.h"

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
