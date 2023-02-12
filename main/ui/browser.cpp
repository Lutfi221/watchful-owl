#include <iostream>
#include <stack>

#include "ftxui/component/screen_interactive.hpp"

#include "browser.h"
#include "config.h"
#include "constants.hpp"
#include "dev-logger.h"
#include "ui/ui.h"

bool Browser::iterate()
{
    auto page = this->pageStack.top();
    INFO("Load `{}` page", page->name);
    NavInstruction n = page->load();
    INFO("Page ended");
    if (n.flag == NavGeneric)
    {
        if (n.nextPage)
        {
            INFO("Navigate to `{}` page", n.nextPage->name);
            this->pageStack.push(n.nextPage);
            return 1;
        }
        if (n.stepsBack != 0)
        {
            INFO("Navigate {} steps back", n.stepsBack);
            for (int i = 0; i < n.stepsBack; i++)
            {
                delete this->pageStack.top();
                this->pageStack.pop();
            }
            return 1;
        }
        throw new std::logic_error("The returned `NavInstruction` is the default value."
                                   "`NavInstruction` have not been modified.");
    }
    if (n.flag == NavExit)
    {
        INFO("Received `NavExit` flag");
        return 0;
    }
    INFO("Navigate to main page");
    for (int i = 1; i < n.stepsBack; i++)
    {
        delete this->pageStack.top();
        this->pageStack.pop();
    }
    return 1;
}

Browser::Browser(ftxui::ScreenInteractive *screen,
                 Page *homePage) : screen(screen)
{
    this->pageStack.push(homePage);
}

void Browser::load()
{
    INFO("Start browser loop");
    while (this->iterate())
        ;
    INFO("Ended browser loop");
}