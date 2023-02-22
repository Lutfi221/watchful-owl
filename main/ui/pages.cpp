#include <string>
#include <vector>

#include "autorun.h"
#include "config.h"
#include "constants.hpp"
#include "crypto.h"
#include "dev-logger.h"
#include "helpers.h"
#include "pages.h"
#include "ui.h"

class AutorunConfigPage : public Page
{
public:
    const std::string name;
    AutorunConfigPage(ftxui::ScreenInteractive *screen, Config *config)
        : Page(screen, config, u8"AutorunConfig"){};

    NavInstruction load();
};
NavInstruction AutorunConfigPage::load()
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

class EncryptionSetUpPage : public Page
{
public:
    const std::string name;
    EncryptionSetUpPage(ftxui::ScreenInteractive *screen, Config *config)
        : Page(screen, config, u8"EncryptionSetUp"){};
    NavInstruction load();
};

NavInstruction EncryptionSetUpPage::load()
{
    Config *config = this->config;
    NavInstruction navInstruction;
    unsigned int sizes[4] = {4096, 2048, 1024, 512};
    std::vector<std::string> entries = {"4096 (best)",
                                        "2048 (recommended by NIST of USA)",
                                        "1024",
                                        "512 (weakest)",
                                        "Back"};
    int s = promptSelection(this->screen,
                            &entries,
                            "RSA Key Size Selection",
                            "How long (in bits) do you want the RSA key to be? "
                            "The longer it is, the more secure, "
                            "but will be (slightly) slower.");

    if (s == 4)
    {
        navInstruction.stepsBack = 1;
        return navInstruction;
    }

    std::string password = promptPassword(
        this->screen, true,
        "Encryption Password",
        "Make a strong password to secure your activity log. "
        "If you forget the password, your activities will be lost forever!");

    unsigned int size = sizes[s];
    auto publicKeyPath = prepareAndProcessPath(config->encryption.rsaPublicKeyPath).u8string();
    auto privateKeyPath = prepareAndProcessPath(config->encryption.rsaPrivateKeyPath).u8string();
    auto saltPath = prepareAndProcessPath(config->encryption.saltPath).u8string();

    crypto::SymKey symKey(password);
    crypto::AsymKey asymKey;
    asymKey.generate(size);

    asymKey.saveToFile(crypto::KeyTypePublic, publicKeyPath);
    asymKey.saveToFile(crypto::KeyTypePrivate, privateKeyPath, &symKey);
    symKey.saveSaltToFile(saltPath);

    this->config->encryption.enabled = true;
    saveConfig(config);

    navInstruction.stepsBack = 1;
    return navInstruction;
};

class EncryptionConfigPage : public Page
{
public:
    const std::string name;
    EncryptionConfigPage(ftxui::ScreenInteractive *screen, Config *config)
        : Page(screen, config, u8"EncryptionConfig"){};
    NavInstruction load();
};
enum EncryptionStatus
{
    EncryptionEnabled,
    EncryptionIncomplete,
    EncryptionDisabled
};
NavInstruction EncryptionConfigPage::load()
{
    Config *config = this->config;
    NavInstruction navInstruction;
    std::string desc;
    EncryptionStatus encryptionStatus;

    std::vector<std::string> entries = {"", "Back"};

    auto publicKeyPath = config->encryption.rsaPublicKeyPath;
    bool keyFileExists = fileExists(publicKeyPath);
    auto encryptionEnabled = config->encryption.enabled;

    if (keyFileExists && encryptionEnabled)
    {
        desc = "Logging encryption is enabled";
        encryptionStatus = EncryptionEnabled;
        entries.at(0) = "Disable logging encryption";
    }
    else if (!keyFileExists && encryptionEnabled)
    {
        desc = "RSA public key is missing, logging encryption is disabled.";
        encryptionStatus = EncryptionIncomplete;
        entries.at(0) = "Generate a new RSA key pair";
    }
    else
    {
        desc = "Logging encryption is disabled";
        encryptionStatus = EncryptionDisabled;
        entries.at(0) = "Enable logging encryption";
    }

    int s = promptSelection(
        this->screen, &entries, "Encryption Configuration", desc);

    switch (s)
    {
    case 0:
        if (encryptionStatus == EncryptionEnabled)
        {
            config->encryption.enabled = false;
            saveConfig(config);
            navInstruction.flag = NavReload;
            break;
        }
        if (keyFileExists)
        {
            config->encryption.enabled = true;
            saveConfig(config);
            navInstruction.flag = NavReload;
            break;
        }

        navInstruction.nextPage = new EncryptionSetUpPage(this->screen, config);
        break;

    default:
        navInstruction.stepsBack = 1;
        break;
    }

    return navInstruction;
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
