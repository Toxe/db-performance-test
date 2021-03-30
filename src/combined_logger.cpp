#include "combined_logger.h"

#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> create_combined_logger(const std::string& logfile_name)
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(logfile_name, false);
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

    auto combined_logger = std::make_shared<spdlog::logger>("combined", sinks.begin(), sinks.end());
    combined_logger->set_level(spdlog::level::info);
    combined_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    spdlog::register_logger(combined_logger);

    return combined_logger;
}
