#ifndef MAIN_UI_BROWSER
#define MAIN_UI_BROWSER
#include <stack>
#include <variant>

#include "ftxui/component/screen_interactive.hpp"

#include "dev-logger.h"
#include "ui/ui.h"

template <typename T>
class Browser
{
private:
    ftxui::ScreenInteractive *screen;
    /// @brief Stack containing either `Page` or `PageFn`.
    std::stack<std::variant<Page *, PageFn>> pageStack;
    bool iterate();
    /// @brief Pop the last page on stack, and delete if necessary.
    void popLastOnStack();
    T *context = nullptr;

public:
    Browser(ftxui::ScreenInteractive *, Page *, T *);
    void load();
};

template <typename T>
bool Browser<T>::iterate()
{
    auto variablePage = this->pageStack.top();
    NavInstruction n;

    if (std::holds_alternative<Page *>(variablePage))
    {
        Page *page = std::get<Page *>(variablePage);
        INFO("Load class page `{}`", page->name);
        n = page->load();
    }
    else
    {
        PageFn page = std::get<PageFn>(variablePage);
        INFO("Load function page");
        n = page(this->screen, this->context);
    }

    INFO("Page ended");
    if (n.flag == NavGeneric)
    {
        if (n.nextPage)
        {
            INFO("Navigate to `{}` page", n.nextPage->name);
            this->pageStack.push(n.nextPage);
            return 1;
        }
        if (n.nextPageFn)
        {
            INFO("Navigate to a function page");
            this->pageStack.push(n.nextPageFn);
            return 1;
        }

        if (n.stepsBack != 0)
        {
            INFO("Navigate {} steps back", n.stepsBack);
            for (int i = 0; i < n.stepsBack; i++)
                this->popLastOnStack();
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

    if (n.flag == NavReload)
        return 1;

    INFO("Navigate to main page");
    for (int i = 1; i < this->pageStack.size(); i++)
        this->popLastOnStack();

    return 1;
}

template <typename T>
Browser<T>::Browser(ftxui::ScreenInteractive *screen,
                    Page *homePage, T *context)
    : screen(screen), context(context)
{
    this->pageStack.push(homePage);
}

template <typename T>
void Browser<T>::load()
{
    INFO("Start browser loop");
    while (this->iterate())
        ;
    INFO("Ended browser loop");
}

template <typename T>
void Browser<T>::popLastOnStack()
{
    auto v = this->pageStack.top();
    if (std::holds_alternative<Page *>(v))
        delete std::get<Page *>(v);

    this->pageStack.pop();
}

#endif /* MAIN_UI_BROWSER */
