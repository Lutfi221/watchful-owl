#ifndef MAIN_UI_UI
#define MAIN_UI_UI
#include <string>
#include <vector>

#include "ftxui/component/screen_interactive.hpp"

#include "config.h"

enum NavFlag
{
    NavGeneric,
    NavReload,
    NavGoHome,
    NavExit
};

struct NavInstruction;

class Page
{
protected:
    ftxui::ScreenInteractive *screen;
    Config *config;

public:
    Page(ftxui::ScreenInteractive *, Config *);
    /// @brief Load page.
    virtual NavInstruction load() = 0;
};

class MainPage : public Page
{
    using MainPage::Page::Page;

public:
    NavInstruction load();
};

struct NavInstruction
{
    NavFlag flag = NavGeneric;
    int stepsBack = 0;
    Page *nextPage = nullptr;
};

#endif /* MAIN_UI_UI */
