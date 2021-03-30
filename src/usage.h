#pragma once

#include <string_view>

#include <clipp.h>

void show_usage_and_exit(const clipp::group& cli, const char* argv0, const std::string_view& description);
