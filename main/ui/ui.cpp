#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "dev-logger.h"
#include "helpers.h"
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
    {
        auto texts = split(description, "\n");
        for (auto &line : texts)
            components.push_back(paragraph(line));
    }
    if (components.size() != 0)
        components.push_back(separatorEmpty());

    components.insert(components.end(), children.begin(), children.end());
    return vbox(components);
};

void showInfo(ftxui::ScreenInteractive *screen,
              std::string title,
              std::string description)
{
    std::vector e({std::string("Ok")});
    promptSelection(screen, &e, title, description);
};

int promptSelection(
    ftxui::ScreenInteractive *screen,
    std::vector<SelectionEntry> *entries,
    std::string title,
    std::string description)
{
    using namespace ftxui;

    int selected = 0;
    MenuOption option;
    option.on_enter = screen->ExitLoopClosure();

    std::vector<std::string> textEntries;

    for (auto &entry : *entries)
        textEntries.push_back(entry.text);

    auto menu = Menu(&textEntries, &selected, &option);
    auto render = [&]()
    {
        auto entry = entries->at(selected);
        std::vector<Element> e;
        e.push_back(menu->Render());
        e.push_back(separatorEmpty());
        e.push_back(paragraph(entry.description));
        if (entry.disabled)
            e.push_back(paragraph(entry.disabilityMessage) | color(Color::Red));

        return basePage(e, title, description);
    };

    auto renderer = Renderer(menu, render);
    do
    {
        screen->Loop(renderer);
    } while (entries->at(selected).disabled);

    INFO("User selected index `{}`, with the option `{}`",
         selected,
         (*entries)[selected].text);
    return selected;
}

int promptSelection(
    ftxui::ScreenInteractive *screen,
    std::vector<std::string> *entries,
    std::string title,
    std::string description)
{
    std::vector<SelectionEntry> e;
    for (auto &entry : *entries)
    {
        e.push_back(SelectionEntry(entry));
    }

    return promptSelection(screen, &e, title, description);
}

bool promptPassword(
    ftxui::ScreenInteractive *screen,
    std::string *output,
    PasswordPromptOption *option)
{
    using namespace ftxui;
    std::string password;
    std::string passwordC;
    bool isFirstAttempt = true;
    bool cancelled = false;

    InputOption passwordOption;
    InputOption passwordCOption;
    passwordOption.password = true;
    passwordCOption.password = true;

    // Password Inputs
    Component passwordInput = Input(&password, "Password", &passwordOption);
    Component passwordCInput = Input(&passwordC, "Confirm Password", &passwordCOption);

    // Submit Button
    Component submitBtn = Button("Submit", screen->ExitLoopClosure());

    // Cancel Button
    auto handleCancel = [&]
    {
        cancelled = true;
        screen->ExitLoopClosure()();
    };
    Component cancelBtn = Button("Cancel", handleCancel);

    Component root = Container::Vertical({passwordInput});

    // Add password confirmation field if needed.
    if (option->withConfirmation)
    {
        root->Add(passwordCInput);
        passwordOption.on_enter = [&]
        { root->SetActiveChild(passwordCInput); };
        passwordCOption.on_enter = screen->ExitLoopClosure();
    }
    else
        passwordOption.on_enter = screen->ExitLoopClosure();

    root->Add(submitBtn);

    if (option->cancellable)
        root->Add(cancelBtn);

    auto render = [&]
    {
        std::vector<Element> content;

        // Input fields
        if (option->withConfirmation)
            content = {hbox(text(" Password : "),
                            passwordInput->Render()),
                       hbox(text(" Confirm  : "),
                            passwordCInput->Render())};
        else
            content = {hbox(text(" Password : "),
                            passwordInput->Render())};

        // Buttons
        content.push_back(submitBtn->Render());
        if (option->cancellable)
            content.push_back(cancelBtn->Render());

        // Extra infos
        if (!isFirstAttempt && password != passwordC)
        {
            content.push_back(separatorEmpty());
            content.push_back(hbox(color(
                Color::Red,
                paragraph("Passwords do not match, try again."))));
        }

        return basePage(content, option->title, option->description);
    };

    auto renderer = Renderer(root, render);
    do
    {
        screen->Loop(renderer);
        isFirstAttempt = false;
    } while (option->withConfirmation && password != passwordC && !cancelled);

    *output = password;
    return !cancelled;
}

bool promptTextInput(
    ftxui::ScreenInteractive *screen,
    std::string *output,
    PromptOption *option)
{
    using namespace ftxui;
    std::vector<Component> components;
    std::string validatorMsg;
    bool validatorMsgVisible = option->validateOnChange;
    bool inputValid = false;
    bool cancelled = false;

    auto handleSubmit = [&]()
    {
        validatorMsgVisible = true;

        if (option->validate == nullptr)
            inputValid = true;
        else
            inputValid = option->validate(output, &validatorMsg);

        if (!inputValid)
            return;

        screen->ExitLoopClosure()();
    };
    auto handleCancel = [&]
    {
        cancelled = true;
        screen->ExitLoopClosure()();
        return;
    };

    InputOption inputOption;
    inputOption.on_enter = handleSubmit;
    if (option->validateOnChange && option->validate != nullptr)
        inputOption.on_change = [&]
        {
            inputValid = option->validate(output, &validatorMsg);
        };

    // Main Input
    Component input = Input(output, option->inputLabel, inputOption);
    components.push_back(input);

    // Submit Button
    Component submitBtn = Button("Submit", handleSubmit);
    components.push_back(submitBtn);

    // Cancel Button
    Component cancelBtn = Button("Cancel", handleCancel);
    if (option->cancellable)
        components.push_back(cancelBtn);

    auto root = Container::Vertical(components);

    auto renderFn = [&]
    {
        std::vector v =
            {
                hbox(
                    text(option->inputLabel + " : "),
                    input->Render()),
                flex_shrink(submitBtn->Render())};

        if (option->cancellable)
            v.push_back(flex_shrink(cancelBtn->Render()));

        if (validatorMsgVisible)
            v.push_back(text(validatorMsg) | color(inputValid ? Color::Green : Color::Red));

        return basePage(v, option->title, option->description);
    };
    auto renderer = Renderer(
        root, renderFn);

    screen->Loop(renderer);
    return !cancelled;
}

Page::Page(ftxui::ScreenInteractive *screen, Config *config, std::string name)
    : screen(screen), config(config), name(name){};
