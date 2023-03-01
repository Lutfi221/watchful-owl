#include <filesystem>
#include <iostream>

#include "dev-logger.h"
#include "ftxui/component/screen_interactive.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include "constants.hpp"
#include "ui/browser.hpp"
#include "ui/pages.h"
#include "ui/ui.h"

using namespace std;

int main(int argc, char **argv)
{
    try
    {
        auto maxSize = 1048576; // 1 megabit
        auto maxFiles = 5;
        auto outputPath = filesystem::weakly_canonical(
                              constants::LOG_OUTPUT_DIR /
                              filesystem::path("./watchful-owl.log"))
                              .u8string();
        auto logger = spdlog::rotating_logger_mt(
            "main", outputPath, maxSize, maxFiles);
        spdlog::set_default_logger(logger);
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("[%l] %v");
        spdlog::flush_every(std::chrono::seconds(3));
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        cout << "Log init failed: " << ex.what() << endl;
        return EXIT_FAILURE;
    }

    try
    {
        auto screen = ftxui::ScreenInteractive::Fullscreen();
        auto config = loadConfig();
        auto mainPage = MainPage(&screen, &config);
        INFO("Initialized main page");

        Browser<Config> browser(&screen, &mainPage, &config);
        INFO("Initialized main browser");

        INFO("Start main browser");
        browser.load();
        INFO("Main browser ended");
    }
    catch (const exception &ex)
    {
        cout << "An error occured.\n"
             << "Open the log file at `"
             << constants::LOG_OUTPUT_DIR
             << "` for more information.\n\n"
             << ex.what() << endl;
        spdlog::error(ex.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
