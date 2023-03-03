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

std::string promptPassword(
    ftxui::ScreenInteractive *screen,
    bool withConfirmation,
    std::string title,
    std::string description)
{
    using namespace ftxui;
    std::string password;
    std::string passwordC;
    bool isFirstAttempt = true;

    InputOption passwordOption;
    InputOption passwordCOption;
    passwordOption.password = true;
    passwordCOption.password = true;

    Component passwordInput = Input(&password, "Password", &passwordOption);
    Component passwordCInput = Input(&passwordC, "Confirm Password", &passwordCOption);

    Component component;
    std::vector<Element> content;

    if (withConfirmation)
    {
        component = Container::Vertical({passwordInput, passwordCInput});
        passwordOption.on_enter = [&]
        { component->SetActiveChild(passwordCInput); };
        passwordCOption.on_enter = screen->ExitLoopClosure();
    }
    else
    {
        component = Container::Vertical({passwordInput});
        passwordOption.on_enter = screen->ExitLoopClosure();
    }

    auto render = [&]
    {
        std::vector<Element> content;

        if (withConfirmation)
            content = {hbox(text(" Password : "),
                            passwordInput->Render()),
                       hbox(text(" Confirm  : "),
                            passwordCInput->Render())};
        else
            content = {hbox(text(" Password : "),
                            passwordInput->Render())};

        if (!isFirstAttempt && password != passwordC)
        {
            content.push_back(separatorEmpty());
            content.push_back(hbox(color(
                Color::Red,
                paragraph("Passwords do not match, try again."))));
        }

        return basePage(content, title, description);
    };

    auto renderer = Renderer(component, render);
    do
    {
        screen->Loop(renderer);
        isFirstAttempt = false;
        if (!withConfirmation)
            break;
    } while (password != passwordC);
    return password;
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
