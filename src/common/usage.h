#pragma once

#include <string_view>

#include <clipp.h>

void show_usage_and_exit(const clipp::group& cli, const std::string_view& argv0, const std::string_view& description, const std::string_view& example);
