#pragma once

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> create_combined_logger(const std::string& logfile_name);
