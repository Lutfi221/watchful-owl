#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "dev-logger.h"
#include "ui.h"

ftxui::Element basePage(
    ftxui::Element child,
    std::string title,
    std::string description)
{
    return basePage(std::vector({child}), title, description);
};

ftxui::Element basePage(
    std::vector<ftxui::Element> children,
    std::string title,
    std::string description)
{
    using namespace ftxui;
    std::vector<Element> components;

    if (title != "")
        components.push_back(color(Color::Cyan, text(title) | bold));
    if (description != "")
        components.push_back(paragraph(description));
    if (components.size() != 0)
        components.push_back(separatorEmpty());

    components.insert(components.end(), children.begin(), children.end());
    return vbox(components);
};

int promptSelection(
    ftxui::ScreenInteractive *screen,
    std::vector<std::string> *entries,
    std::string title,
    std::string description)
{
    using namespace ftxui;

    int selected = 0;
    MenuOption option;
    option.on_enter = screen->ExitLoopClosure();

    auto menu = Menu(entries, &selected, &option);
    auto render = [&]()
    {
        return basePage(menu->Render(), title, description);
    };

    auto renderer = Renderer(menu, render);
    screen->Loop(renderer);
    INFO("User selected index `{}`, with the option `{}`",
         selected,
         (*entries)[selected]);
    return selected;
}

Page::Page(ftxui::ScreenInteractive *screen, Config *config, std::string name)
    : screen(screen), config(config), name(name){};
