#ifndef MAIN_UI_PAGES
#define MAIN_UI_PAGES
#include "ftxui/component/screen_interactive.hpp"
#include "ui.h"

class MainPage : public Page
{
public:
    std::string name;
    MainPage(ftxui::ScreenInteractive *, Config *);
    NavInstruction load();
};

#endif /* MAIN_UI_PAGES */
