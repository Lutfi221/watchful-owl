#include <memory>
#include <string>
#include <vector>

#include "autorun.h"
#include "config.h"
#include "constants.hpp"
#include "crypto.h"
#include "dev-logger.h"
#include "helpers.h"
#include "logger.h"
#include "pages.h"
#include "ui.h"

#include "git.h"

#define DEFAULT_DECRYPTED_DEST_DIR "./decrypted-logs"
#define SLEEP_LEN_AFTER_INSTANCE_KILL 0.3f

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
                      " - The Watchful Owl program was moved to a different "
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
    NavInstruction navInstruction;
    navInstruction.stepsBack = 1;

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
        return navInstruction;

    std::string password;
    PasswordPromptOption promptOption;
    promptOption.withConfirmation = true;
    promptOption.cancellable = true;
    promptOption.title = "Encryption Password Set Up";
    promptOption.description =
        "Make a strong password to secure your activity log. \n"
        "If you forget the password, your activities will be lost forever!";

    if (!promptPassword(this->screen, &password, &promptOption))
        return navInstruction;

    unsigned int size = sizes[s];
    auto publicKeyPath = prepareAndProcessPath(config->encryption.rsaPublicKeyPath).u8string();
    auto privateKeyPath = prepareAndProcessPath(config->encryption.rsaPrivateKeyPath).u8string();
    auto saltPath = prepareAndProcessPath(config->encryption.saltPath).u8string();

    crypto::SymKeyPasswordBased symKey(password);
    crypto::AsymKey asymKey;
    asymKey.generate(size);

    asymKey.saveToFile(crypto::KeyTypePublic, publicKeyPath);
    asymKey.saveToFile(crypto::KeyTypePrivate, privateKeyPath, &symKey);
    symKey.saveSaltToFile(saltPath);

    this->config->encryption.enabled = true;
    saveConfig(config);

    return navInstruction;
};

NavInstruction LogDecryptionPage(ftxui::ScreenInteractive *screen, Config *config)
{
    NavInstruction navInstruction;
    navInstruction.stepsBack = 1;

    std::string saltPath =
        prepareAndProcessPath(config->encryption.saltPath)
            .u8string();
    std::string privateKeyPath =
        prepareAndProcessPath(config->encryption.rsaPrivateKeyPath)
            .u8string();

    std::unique_ptr<crypto::SymKeyPasswordBased> symKey(nullptr);
    std::unique_ptr<crypto::AsymKey> asymKey(nullptr);
    std::string password;
    bool asymKeyLoaded = false;

    PasswordPromptOption passPromptOption;
    passPromptOption.cancellable = true;
    passPromptOption.title = "Private Key Password";
    passPromptOption.description = "Enter your secret private key password.";

    while (!asymKeyLoaded)
    {
        password = "";
        asymKey.reset(new crypto::AsymKey);
        if (!promptPassword(screen, &password, &passPromptOption))
            return navInstruction;

        INFO("Generate symmetric key from user's password");
        ftxui::Render(
            *screen,
            basePage(
                ftxui::emptyElement(),
                "Processing...",
                "Processing the user's password, please wait."));
        screen->Print();

        symKey.reset(new crypto::SymKeyPasswordBased(password, saltPath));

        try
        {
            INFO("Load RSA private key from {}", saltPath);
            asymKey->loadFromFile(
                crypto::KeyTypePrivate,
                privateKeyPath,
                symKey.get());
            asymKeyLoaded = true;
        }
        catch (const crypto::DecryptionError &ex)
        {
            SPDERROR(ex.what());

            std::vector<std::string> e({"Try again", "Cancel"});
            int i = promptSelection(screen,
                                    &e,
                                    "Decryption Error",
                                    "Cannot decrypt RSA private key.\n"
                                    "You might have entered an incorrect password, "
                                    "or the encrypted private key data is corrupted.");
            if (i == 1)
                return navInstruction;
        }
    }

    DEBUG("Validate RSA private key");
    if (!asymKey->validate(crypto::KeyTypePrivate))
    {
        INFO("RSA private key is invalid");
        showInfo(
            screen,
            "Invalid Private Key",
            "Loaded RSA private key is invalid.\n"
            "Path: `" +
                privateKeyPath + "`");
        return navInstruction;
    }

    INFO("RSA private key is valid");

    std::string sourceDir = config->outDir;
    std::string destDir = DEFAULT_DECRYPTED_DEST_DIR;
    bool cancelled = false;

    // Prompt for source directory
    PromptOption promptOption;
    promptOption.inputLabel = "Source Directory";
    promptOption.title = "Source Directory Path";
    promptOption.description = "Input the directory path containing "
                               "the encrypted log files.";

    if (!promptTextInput(screen, &sourceDir, &promptOption))
        return navInstruction;
    sourceDir = prepareAndProcessPath(sourceDir, true, true).u8string();

    // Prompt for destination directory
    promptOption.inputLabel = "Destination Directory";
    promptOption.title = "Destination Directory Path";
    promptOption.description = "Input the destination directory path to put "
                               "the decrypted log files.";

    if (!promptTextInput(screen, &destDir, &promptOption))
        return navInstruction;
    destDir = prepareAndProcessPath(destDir, true, true).u8string();

    ftxui::Render(
        *screen,
        basePage(ftxui::emptyElement(), "Processing...", "Decrypting Log Files..."));
    screen->Print();

    INFO("Decrypt log files from `{}` to `{}`", sourceDir, destDir);
    logger::decryptLogFiles(sourceDir, destDir, asymKey.get());
    screen->Clear();

    showInfo(screen,
             "Decryption Complete",
             "Log files from `" + sourceDir +
                 "` has been decrypted to `" + destDir + "`.");

    return navInstruction;
}

class EncryptionPage : public Page
{
public:
    const std::string name;
    EncryptionPage(ftxui::ScreenInteractive *screen, Config *config)
        : Page(screen, config, u8"EncryptionConfig"){};
    NavInstruction load();
};
enum EncryptionStatus
{
    EncryptionEnabled,
    EncryptionIncomplete,
    EncryptionDisabled
};
NavInstruction EncryptionPage::load()
{
    Config *config = this->config;
    NavInstruction navInstruction;
    std::string desc;
    EncryptionStatus encryptionStatus;

    std::vector<std::string> entries = {"", "Decrypt Logs", "Back"};

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

    case 1:
        navInstruction.nextPageFn = &LogDecryptionPage;
        break;

    default:
        navInstruction.stepsBack = 1;
        break;
    }

    return navInstruction;
};

NavInstruction InfoPage(ftxui::ScreenInteractive *screen, Config *config)
{
    INFO("Load `InfoPage`");
    std::string hash(git_CommitSHA1());
    std::string title = "Watchful Owl Information";
    std::string desc = "Watchful Owl (Opened Windows Logger) is an app that logs "
                       "your active and opened windows throughout the day.\n"
                       " \n"
                       "Version: " PROJECT_VERSION "\n"
                       "Commit : " +
                       hash + "\n" +
                       " \n"
                       "Homepage at https://github.com/Lutfi221/watchful-owl\n"
                       "Created by Lutfi Azis";

    ftxui::ButtonOption btnOption;
    auto btn = ftxui::Button("Go Back", screen->ExitLoopClosure(), btnOption.Ascii());

    auto render = [&]()
    {
        return basePage(btn->Render(), title, desc);
    };

    auto renderer = ftxui::Renderer(btn, render);
    screen->Loop(renderer);

    NavInstruction n;
    n.stepsBack = 1;
    return n;
}

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

    std::vector<SelectionEntry> entries = {
        isInstanceRunning
            ? SelectionEntry("Deactivate Watchful Owl")
            : SelectionEntry("Activate Watchful Owl"),
        SelectionEntry("Configure Autorun"),
        SelectionEntry("Encryption",
                       "",
                       isInstanceRunning,
                       "Deactivate Watchful Owl first before opening the encryption page."),
        SelectionEntry("About Watchful Owl"),
        SelectionEntry("Exit")};

    int selection = promptSelection(screen, &entries, "Main Menu", desc);

    std::string perpetualExePath;
    switch (selection)
    {
    case 0:
        /* activate or deactivate perpetual owl */
        if (isInstanceRunning)
        {
            killAllPerpetualInstances();
            // To give some time for Windows to kill the perpetual instance.
            sleepFor(SLEEP_LEN_AFTER_INSTANCE_KILL);
        }
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
        navInstruction.nextPage = new EncryptionPage(this->screen, this->config);
        break;
    case 3:
        navInstruction.nextPageFn = &InfoPage;
        break;
    case 4:
        navInstruction.flag = NavExit;
        break;
    default:
        navInstruction.flag = NavReload;
        break;
    }
    return navInstruction;
}
