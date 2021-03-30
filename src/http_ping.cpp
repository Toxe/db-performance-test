#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>

#include <clipp.h>
#include <cpr/cpr.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

void ping(const std::string& url)
{
    const auto res = cpr::Get(cpr::Url{url});

    spdlog::info("status_code: {}", res.status_code);
}

void show_usage_and_exit(const clipp::group& cli, const char* argv0)
{
    std::string progname{std::filesystem::path{argv0}.filename().string()};
    fmt::print("{}", clipp::make_man_page(cli, progname).prepend_section("DESCRIPTION", "    Ping a URL."));

    std::exit(1);
}

auto eval_args(int argc, char* argv[])
{
    bool show_help = false;
    auto log_level = spdlog::level::warn;
    std::string url;

    auto cli = (
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show verbose output",
        clipp::value("host", url)
            % "URL to ping"
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0]);

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);

    if (show_help)
        show_usage_and_exit(cli, argv[0]);

    return std::make_tuple(url);
}

int main(int argc, char* argv[])
{
    auto [url] = eval_args(argc, argv);

    ping(url);
}
