#include <iostream>
#include "ftxui/component/screen_interactive.hpp"

#include "ui/ui.h"
#include "config.h"

using namespace std;

int main(int argc, char **argv)
{
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    vector<string> entries = {"Activate Watchful Owl", "Enable Autorun", "Exit"};
    int s = promptSelection(&screen, &entries, "Main Menu", "Watchful Owl is currently INACTIVE.");

    switch (s)
    {
    case 0:
        /* activate watchful owl */
        break;
    case 1:
        /* enable autorun */
        break;
    default:
        return 0;
        break;
    }
}
