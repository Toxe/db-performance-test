#include <chrono>
#include <csignal>
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

#include "common/combined_logger.h"
#include "common/msg.h"
#include "common/statistics.h"
#include "common/usage.h"

using namespace std::chrono_literals;

bool running = true;

void signal_handler(int signal)
{
    if (signal == SIGINT) {
        fmt::print("\n");
        running = false;
    }
}

auto continuously_send_pings(cpr::Session& sess, std::chrono::seconds interval)
{
    spdlog::info("sending messages to {}...", sess.Get().url);

    int num_errors = 0;
    std::vector<float> durations;

    while (running) {
        const auto res = msg(sess, "performance.ping", {});

        if (res.has_value() && res->status == 0) {
            spdlog::get("combined")->info("{} --> {:.0f}ms", sess.Get().url, res->elapsed);
            durations.push_back(res->elapsed);
        } else {
            ++num_errors;
        }

        std::this_thread::sleep_for(interval);
    }

    return std::make_tuple(durations, num_errors);
}

auto eval_args(int argc, char* argv[])
{
    const auto description = "Send ping messages.";
    const auto example = "https://example.com user password";
    bool show_help = false;
    auto log_level = spdlog::level::warn;
    int interval = 1;
    int timeout = 30000;
    std::string url;
    std::string user;
    std::string password;
    std::string logfile_name{"logs/msg_ping.log"};

    auto cli = (
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show verbose output",
        clipp::value("host", url)
            % "Host URL",
        clipp::value("user", user)
            % "Login user name",
        clipp::value("password", password)
            % "Login password",
        (clipp::option("--log") & clipp::value("logfile", logfile_name))
            % fmt::format("logfile name (default: {})", logfile_name),
        (clipp::option("--interval") & clipp::integer("interval", interval))
            % fmt::format("wait \"interval\" seconds between each request (default: {}s)", interval),
        (clipp::option("--timeout") & clipp::integer("timeout", timeout))
            % fmt::format("request timeout in milliseconds (default: {}ms)", timeout)
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0], description, example);

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);
    spdlog::info("command line option \"user\": {}", user);
    spdlog::info("command line option \"password\": ???");
    spdlog::info("command line option --log: {}", logfile_name);
    spdlog::info("command line option --interval: {}s", interval);
    spdlog::info("command line option --timeout: {}ms", timeout);

    if (show_help)
        show_usage_and_exit(cli, argv[0], description, example);

    return std::make_tuple(url, user, password, logfile_name, std::chrono::seconds{interval}, std::chrono::milliseconds{timeout});
}

int main(int argc, char* argv[])
{
    const auto [url, user, password, logfile_name, interval, timeout] = eval_args(argc, argv);

    std::signal(SIGINT, signal_handler);
    create_combined_logger(logfile_name);

    auto sess = msg_login(url, user, password, timeout);
    const auto [durations, num_errors] = continuously_send_pings(sess, interval);
    msg_logout(sess);

    show_stats(url, durations, num_errors);
}
