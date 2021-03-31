#include <chrono>
#include <csignal>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <clipp.h>
#include <cpr/cpr.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "combined_logger.h"
#include "statistics.h"
#include "usage.h"

using namespace std::chrono_literals;

bool running = true;

void signal_handler(int signal)
{
    if (signal == SIGINT) {
        fmt::print("\n");
        running = false;
    }
}

std::optional<float> ping(const std::string& url, std::chrono::milliseconds timeout)
{
    const auto r = cpr::Get(cpr::Url{url}, cpr::Timeout{timeout});

    if (r.status_code == 200)
        return {1000.0f * static_cast<float>(r.elapsed)};

    if (r.status_code > 0)
        spdlog::get("combined")->warn(r.status_line);
    else
        spdlog::get("combined")->error(r.error.message);

    return {};
}

auto continuously_send_pings(const std::string& url, std::chrono::seconds interval, std::chrono::milliseconds timeout)
{
    spdlog::info("pinging {}...", url);

    int num_errors = 0;
    std::vector<float> durations;

    while (running) {
        auto ms = ping(url, timeout);

        if (ms.has_value()) {
            spdlog::get("combined")->info("{:.0f}ms", ms.value());
            durations.push_back(ms.value());
        } else {
            ++num_errors;
        }

        std::this_thread::sleep_for(interval);
    }

    return std::make_tuple(durations, num_errors);
}

auto eval_args(int argc, char* argv[])
{
    bool show_help = false;
    auto log_level = spdlog::level::warn;
    int interval = 1;
    int timeout = 30000;
    std::string url;
    std::string logfile_name{"logs/http_ping.log"};

    auto cli = (
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show verbose output",
        clipp::value("host", url)
            % "URL to ping",
        (clipp::option("--log") & clipp::value("logfile", logfile_name))
            % fmt::format("logfile name (default: {})", logfile_name),
        (clipp::option("--interval") & clipp::integer("interval", interval))
            % fmt::format("wait \"interval\" seconds between each request (default: {}s)", interval),
        (clipp::option("--timeout") & clipp::integer("timeout", timeout))
            % fmt::format("request timeout in milliseconds (default: {}ms)", timeout)
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0], "Ping a URL.", "https://example.com");

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);
    spdlog::info("command line option --log: {}", logfile_name);
    spdlog::info("command line option --interval: {}s", interval);
    spdlog::info("command line option --timeout: {}ms", timeout);

    if (show_help)
        show_usage_and_exit(cli, argv[0], "Ping a URL.", "https://example.com");

    return std::make_tuple(url, logfile_name, std::chrono::seconds{interval}, std::chrono::milliseconds{timeout});
}

int main(int argc, char* argv[])
{
    auto [url, logfile_name, interval, timeout] = eval_args(argc, argv);

    std::signal(SIGINT, signal_handler);
    create_combined_logger(logfile_name);

    const auto [durations, num_errors] = continuously_send_pings(url, interval, timeout);

    show_stats(durations, num_errors);
}
