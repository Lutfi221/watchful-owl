#include <string>
#include <vector>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "ui.h"

ftxui::Element BasePage(ftxui::Element child, std::string title, std::string description)
{
    using namespace ftxui;
    std::vector<Element> components;
    if (title != "")
        components.push_back(color(Color::Cyan, text(title) | bold));
    if (description != "")
        components.push_back(text(description));
    if (components.size() != 0)
        components.push_back(separatorEmpty());
    components.push_back(child);
    return vbox(components);
};

/// @brief Display a menu of options, and return the index of selected option.
/// @param entries List of options
/// @param title Title of the menu
/// @param description Description of the menu
/// @return
int promptSelection(ftxui::ScreenInteractive *screen, std::vector<std::string> *entries, std::string title, std::string description)
{
    using namespace ftxui;

    int selected = 0;
    MenuOption option;
    option.on_enter = screen->ExitLoopClosure();

    auto menu = Menu(entries, &selected, &option);
    auto render = [&]()
    {
        return BasePage(menu->Render(), title, description);
    };

    auto renderer = Renderer(menu, render);
    screen->Loop(renderer);
    return selected;
}
