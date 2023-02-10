#include <stack>
#include <iostream>

#include "ftxui/component/screen_interactive.hpp"

#include "constants.hpp"
#include "ui/ui.h"
#include "config.h"
#include "browser.h"

bool Browser::iterate()
{
    NavInstruction n = this->pageStack.top()->load();
    if (n.flag == NavGeneric)
    {
        if (n.nextPage)
        {
            this->pageStack.push(n.nextPage);
            return 1;
        }
        if (n.stepsBack != 0)
        {
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
        return 0;
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
    while (this->iterate())
        ;
}