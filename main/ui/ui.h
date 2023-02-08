#ifndef MAIN_UI_UI
#define MAIN_UI_UI
#include <string>
#include <vector>
#include "ftxui/component/screen_interactive.hpp"

int promptSelection(ftxui::ScreenInteractive *screen,
                    std::vector<std::string> *entries,
                    std::string title = "",
                    std::string description = "");

#endif /* MAIN_UI_UI */
