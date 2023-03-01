#ifndef MAIN_UI_UI
#define MAIN_UI_UI
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
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

/// @brief A page in function form.
typedef NavInstruction (*PageFn)(ftxui::ScreenInteractive *, Config *);

class Page
{
protected:
    ftxui::ScreenInteractive *screen;
    Config *config;

public:
    std::string name;
    Page(ftxui::ScreenInteractive *, Config *, std::string);
    /// @brief Load page.
    virtual NavInstruction load() = 0;
};

struct NavInstruction
{
    NavFlag flag = NavGeneric;
    int stepsBack = 0;
    Page *nextPage = nullptr;
    PageFn nextPageFn = nullptr;
};

/// @brief Standard page layout.
/// @param children Page content
/// @param title Page title
/// @param description Page description
/// @return An ftxui element
ftxui::Element basePage(
    std::vector<ftxui::Element> children,
    std::string title,
    std::string description);
ftxui::Element basePage(
    ftxui::Element child,
    std::string title,
    std::string description);

/// @brief Display a menu of options, and return the index of selected option.
/// @param entries List of options
/// @param title Title of the menu
/// @param description Description of the menu
/// @return Index of selected option (zero-based)
int promptSelection(
    ftxui::ScreenInteractive *screen,
    std::vector<std::string> *entries,
    std::string title,
    std::string description);

/// @brief Prompt user for a password
/// @param screen FTXUI screen
/// @param withConfirmation Display an extra password input as confirmation
/// @param title Title
/// @param description Description
/// @return The inputted password
std::string promptPassword(
    ftxui::ScreenInteractive *screen,
    bool withConfirmation = false,
    std::string title = "",
    std::string description = "");

#endif /* MAIN_UI_UI */
