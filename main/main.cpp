#include <filesystem>
#include <iostream>

#include "dev-logger.h"
#include "ftxui/component/screen_interactive.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include "constants.hpp"
#include "ui/browser.h"
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
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto config = loadConfig();
    auto mainPage = MainPage(&screen, &config);
    INFO("Initialized main page");

    Browser browser = Browser(&screen, &mainPage);
    INFO("Initialized main browser");

    INFO("Start main browser");
    browser.load();
    INFO("Main browser ended");
    return EXIT_SUCCESS;
}
