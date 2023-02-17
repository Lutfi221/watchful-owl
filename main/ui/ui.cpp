#include <filesystem>
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "autorun.h"
#include "constants.hpp"
#include "dev-logger.h"
#include "helpers.h"
#include "ui.h"

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
    INFO("User selected index `{}`, with the option `{}`",
         selected,
         (*entries)[selected]);
    return selected;
}

Page::Page(ftxui::ScreenInteractive *screen, Config *config, std::string name)
    : screen(screen), config(config), name(name){};

class AutorunConfigPage : public Page
{
public:
    const std::string name;
    AutorunConfigPage(ftxui::ScreenInteractive *screen, Config *config)
        : Page(screen, config, u8"AutorunConfig"){};

    NavInstruction load()
    {
        NavInstruction navInstruction;
        AutorunStatus autorunStatus = getAutorunStatus();
        std::string description;
        std::vector<std::string> entries = {"", "Back"};
        switch (autorunStatus)
        {
        case AutorunDisabled:
            INFO("Autorun is disabled");
            entries[0] = "Enable Autorun";
            description = "Autorun is NOT enabled for the current user. "
                          "Watchful Owl will NOT start automatically "
                          "when the computer turns on.";
            break;
        case AutorunInvalid:
            INFO("Autorun is invalid");
            entries[0] = "Fix Invalid Autorun and Enable It";
            description = "Autorun is NOT VALID.\n"
                          "This might be caused by the following reasons:\n"
                          " - The Watchful Owl program was moved to a different"
                          "location since it was opened.\n"
                          " - There are two Watchful Owl program in this PC.\n\n"
                          "Would you like to fix the issue?";
            break;
        case AutorunEnabled:
            INFO("Autorun is enabled");
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

class EncryptionConfigPage : public Page
{
public:
    const std::string name;
    EncryptionConfigPage(ftxui::ScreenInteractive *screen, Config *config)
        : Page(screen, config, u8"EncryptionConfig"){};

    NavInstruction load()
    {
        Config *config = this->config;
        NavInstruction navInstruction;
        auto publicKeyPath = config->encryption.rsaPublicKeyPath;
        auto encryptionEnabled = config->encryption.enabled;

        std::vector<std::string> entries = {
            encryptionEnabled
                ? "Disable Logging Encryption Temporarily"
                : "Enable Logging Encryption",
            "Back"};

        int s = promptSelection(
            screen, &entries, "Encryption Configuration",
            "Log file encryption prevents other "
            "people from accessing your log files.");

        switch (s)
        {
        case 0:
            navInstruction.flag = NavReload;
            if (encryptionEnabled)
            {
                config->encryption.enabled = false;
                saveConfig(config);
                break;
            }

            config->encryption.enabled = true;
            saveConfig(config);
            break;

        default:
            navInstruction.stepsBack = 1;
            break;
        }

        return navInstruction;
    };
};

MainPage::MainPage(ftxui::ScreenInteractive *screen, Config *config)
    : Page(screen, config, u8"Main"){};

NavInstruction MainPage::load()
{
    namespace fs = std::filesystem;
    NavInstruction navInstruction;
    bool isInstanceRunning = isPerpetualInstanceRunning();
    INFO("isInstanceRunning={}", isInstanceRunning);

    std::string desc = isInstanceRunning
                           ? "Watchful Owl is ACTIVE and currently logging your activity."
                           : "Watchful Owl is currently INACTIVE.";
    std::vector<std::string> entries = {
        isInstanceRunning
            ? "Deactivate Watchful Owl"
            : "Activate Watchful Owl",
        "Configure Autorun",
        "Configure Encryption",
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
        navInstruction.nextPage = new EncryptionConfigPage(this->screen, this->config);
        break;
    case 3:
        navInstruction.flag = NavExit;
        break;
    default:
        navInstruction.flag = NavReload;
        break;
    }
    return navInstruction;
}
