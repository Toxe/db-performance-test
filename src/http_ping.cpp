#include <chrono>
#include <csignal>
#include <filesystem>
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

void ping(const std::string& url)
{
    const auto r = cpr::Get(cpr::Url{url});

    if (r.status_code == 200)
        spdlog::get("ping")->info("{:.0f} ms", r.elapsed * 1000.0);
    else if (r.status_code > 0)
        spdlog::get("ping")->warn(r.status_line);
    else
        spdlog::get("ping")->error(r.error.message);
}

void continuously_ping_url(const std::string& url)
{
    spdlog::info("pinging {}...", url);

    while (running) {
        ping(url);
        std::this_thread::sleep_for(1s);
    }
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
    std::string logfile_name{"logs/http_ping.log"};

    auto cli = (
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show verbose output",
        clipp::value("host", url)
            % "URL to ping",
        (clipp::option("--log") & clipp::value("logfile", logfile_name))
            % fmt::format("logfile name (default: {})", logfile_name)
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0]);

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);
    spdlog::info("command line option --log: {}", logfile_name);

    if (show_help)
        show_usage_and_exit(cli, argv[0]);

    return std::make_tuple(url, logfile_name);
}

int main(int argc, char* argv[])
{
    auto [url, logfile_name] = eval_args(argc, argv);

    std::signal(SIGINT, signal_handler);
    create_ping_logger(logfile_name);

    continuously_ping_url(url);
}
