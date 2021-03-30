#include "usage.h"

#include <filesystem>
#include <string>

#include <fmt/core.h>
#include <fmt/ostream.h>

void show_usage_and_exit(const clipp::group& cli, const std::string_view& argv0, const std::string_view& description)
{
    std::string progname{std::filesystem::path{argv0}.filename().string()};
    fmt::print("{}", clipp::make_man_page(cli, progname).prepend_section("DESCRIPTION", fmt::format("    {}", description)));

    std::exit(1);
}
