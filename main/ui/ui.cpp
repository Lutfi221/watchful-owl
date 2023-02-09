#include <string>
#include <vector>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "ui.h"
#include "autorun.h"
#include "constants.hpp"
#include <filesystem>

ftxui::Element BasePage(ftxui::Element child, std::string title, std::string description)
{
    using namespace ftxui;
    std::vector<Element> components;
    if (title != "")
        components.push_back(color(Color::Cyan, text(title) | bold));
    if (description != "")
        components.push_back(paragraph(description));
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

void autorunConfigPage(ftxui::ScreenInteractive *screen)
{
    while (true)
    {
        AutorunStatus autorunStatus = getAutorunStatus();
        std::string description;
        std::vector<std::string> entries = {"", "Back"};
        switch (autorunStatus)
        {
        case AutorunDisabled:
            entries[0] = "Enable Autorun";
            description = "Autorun is NOT enabled for the current user. "
                          "Watchful Owl will NOT start automatically "
                          "when the computer turns on.";
            break;
        case AutorunInvalid:
            entries[0] = "Fix Invalid Autorun and Enable It";
            description = "Autorun is NOT VALID.\n"
                          "This might be caused by the following reasons:\n"
                          " - The Watchful Owl program was moved to a different"
                          "location since it was opened.\n"
                          " - There are two Watchful Owl program in this PC.\n\n"
                          "Would you like to fix the issue?";
            break;
        case AutorunEnabled:
            entries[0] = "Disable Autorun";
            description = "Autorun is ENABLED for the current user. "
                          "Watchful Owl will automatically "
                          "start logging when the computer turns on.";
            break;
        default:
            break;
        }

        auto s = promptSelection(screen, &entries, "Autorun Configuration", description);

        if (s == 1)
            return;

        if (autorunStatus == AutorunDisabled || autorunStatus == AutorunInvalid)
            enableAutorun();
        else
            disableAutorun();
    }
    return;
}