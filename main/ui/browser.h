#ifndef MAIN_UI_BROWSER
#define MAIN_UI_BROWSER

#include <stack>
#include "ftxui/component/screen_interactive.hpp"
#include "ui/ui.h"
#include "config.h"

class Browser
{
private:
    ftxui::ScreenInteractive *screen;
    std::stack<Page *> pageStack;
    bool iterate();

public:
    Browser(ftxui::ScreenInteractive *, Page *);
    void load();
};

#endif /* MAIN_UI_BROWSER */
