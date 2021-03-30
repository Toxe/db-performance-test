#include <algorithm>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <numeric>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <clipp.h>
#include <cpr/cpr.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace std::chrono_literals;

bool running = true;

void signal_handler(int signal)
{
    if (signal == SIGINT) {
        fmt::print("\n");
        running = false;
    }
}

std::shared_ptr<spdlog::logger> create_ping_logger(const std::string& logfile_name)
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    console_sink->set_level(spdlog::level::info);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(logfile_name, false);
    file_sink->set_level(spdlog::level::info);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("ping", sinks.begin(), sinks.end());

    spdlog::register_logger(logger);

    return logger;
}

double mean(const std::vector<double>& values)
{
    if (values.empty())
        return 0.0;

    return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
}

double median(const std::vector<double>& values)
{
    if (values.empty())
        return 0.0;

    std::vector<double> sorted_values{values};
    std::sort(sorted_values.begin(), sorted_values.end());

    if (sorted_values.size() % 2)
        return sorted_values[(sorted_values.size() - 1) / 2];
    else
        return (sorted_values[sorted_values.size() / 2 - 1] + sorted_values[sorted_values.size() / 2]) / 2.0;
}

std::optional<double> ping(const std::string& url, std::chrono::milliseconds timeout)
{
    const auto r = cpr::Get(cpr::Url{url}, cpr::Timeout{timeout});

    if (r.status_code == 200) {
        spdlog::get("ping")->info("{:.0f}ms", r.elapsed * 1000.0);
        return {r.elapsed};
    }

    if (r.status_code > 0)
        spdlog::get("ping")->warn(r.status_line);
    else
        spdlog::get("ping")->error(r.error.message);

    return {};
}

void continuously_ping_url(const std::string& url, std::chrono::seconds interval, std::chrono::milliseconds timeout)
{
    spdlog::info("pinging {}...", url);

    int errors = 0;
    std::vector<double> durations;

    while (running) {
        auto ms = ping(url, timeout);

        if (ms.has_value())
            durations.push_back(ms.value());
        else
            ++errors;

        std::this_thread::sleep_for(interval);
    }

    fmt::print("pings successful: {}, errors: {}\n", durations.size(), errors);
    fmt::print("mean: {:.2f}ms\n", 1000.0 * mean(durations));
    fmt::print("median: {:.2f}ms\n", 1000.0 * median(durations));
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
        show_usage_and_exit(cli, argv[0]);

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);
    spdlog::info("command line option --log: {}", logfile_name);
    spdlog::info("command line option --interval: {}s", interval);
    spdlog::info("command line option --timeout: {}ms", timeout);

    if (show_help)
        show_usage_and_exit(cli, argv[0]);

    return std::make_tuple(url, logfile_name, std::chrono::seconds{interval}, std::chrono::milliseconds{timeout});
}

int main(int argc, char* argv[])
{
    auto [url, logfile_name, interval, timeout] = eval_args(argc, argv);

    std::signal(SIGINT, signal_handler);
    create_ping_logger(logfile_name);

    continuously_ping_url(url, interval, timeout);
}
