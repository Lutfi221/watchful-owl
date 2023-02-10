#include <string>
#include <vector>

#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "ui.h"
#include "autorun.h"
#include "constants.hpp"
#include "helpers.h"
#include <filesystem>

ftxui::Element createBasePageElement(
    ftxui::Element child,
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
    components.push_back(child);
    return vbox(components);
};

/// @brief Display a menu of options, and return the index of selected option.
/// @param entries List of options
/// @param title Title of the menu
/// @param description Description of the menu
/// @return
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
        return createBasePageElement(menu->Render(), title, description);
    };

    auto renderer = Renderer(menu, render);
    screen->Loop(renderer);
    return selected;
}

Page::Page(ftxui::ScreenInteractive *screen, Config *config)
    : screen(screen), config(config){};

class AutorunConfigPage : public Page
{
public:
    using AutorunConfigPage::Page::Page;

    NavInstruction load()
    {
        NavInstruction navInstruction;
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
        auto s = promptSelection(this->screen, &entries, "Autorun Configuration", description);

        if (s == 1)
        {
            navInstruction.stepsBack = 1;
            return navInstruction;
        }

        navInstruction.flag = NavReload;
        if (autorunStatus == AutorunDisabled || autorunStatus == AutorunInvalid)
            enableAutorun();
        else
            disableAutorun();
        return navInstruction;
    }
};

NavInstruction MainPage::load()
{
    namespace fs = std::filesystem;
    NavInstruction navInstruction;
    bool isInstanceRunning = isPerpetualInstanceRunning();
    std::string desc = isInstanceRunning
                           ? "Watchful Owl is ACTIVE and currently logging your activity."
                           : "Watchful Owl is currently INACTIVE.";
    std::vector<std::string> entries = {
        isInstanceRunning
            ? "Deactivate Watchful Owl"
            : "Activate Watchful Owl",
        "Configure Autorun",
        "Exit"};

    int selection = promptSelection(screen, &entries, "Main Menu", desc);

    std::string perpetualExePath;
    switch (selection)
    {
    case 0:
        /* activate or deactivate perpetual owl */
        if (isInstanceRunning)
            killAllPerpetualInstances();
        else
        {
            perpetualExePath = (getExecutableDirPath() /
                                fs::path(constants::PERPETUAL_EXE_FILENAME))
                                   .u8string();
            startProgram(perpetualExePath);
        }
        navInstruction.flag = NavReload;
        break;
    case 1:
        /* go to autorun config page */
        navInstruction.nextPage = new AutorunConfigPage(this->screen, this->config);
        break;
    case 2:
        navInstruction.flag = NavExit;
        break;
    default:
        navInstruction.flag = NavReload;
        break;
    }
    return navInstruction;
}
