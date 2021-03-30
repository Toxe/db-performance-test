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

float mean(const std::vector<float>& values)
{
    if (values.empty())
        return 0.0f;

    return std::accumulate(values.begin(), values.end(), 0.0f) / static_cast<float>(values.size());
}

float median(const std::vector<float>& values)
{
    if (values.empty())
        return 0.0f;

    std::vector<float> sorted_values{values};
    std::sort(sorted_values.begin(), sorted_values.end());

    if (sorted_values.size() % 2)
        return sorted_values[(sorted_values.size() - 1) / 2];
    else
        return (sorted_values[sorted_values.size() / 2 - 1] + sorted_values[sorted_values.size() / 2]) / 2.0f;
}

std::optional<float> msg(cpr::Session& sess, const std::string& fqmn, std::vector<cpr::Pair> data)
{
    data.push_back({"msg", fqmn});
    sess.SetPayload(cpr::Payload{data.begin(), data.end()});

    const auto r = sess.Post();

    if (r.status_code != 200) {
        if (r.status_code > 0)
            spdlog::get("ping")->warn(r.status_line);
        else
            spdlog::get("ping")->error(r.error.message);

        return {};
    }

    const auto json = nlohmann::json::parse(r.text);

    if (json["status"] != 0) {
        if (json["status"] > 0)
            spdlog::get("ping")->warn("{} ({})", json["status_msg"], json["status"]);
        else
            spdlog::get("ping")->error("{} ({})", json["status_msg"], json["status"]);

        return {};
    }

    return {1000.0f * static_cast<float>(r.elapsed)};
}

auto continuously_send_pings(cpr::Session& sess, std::chrono::seconds interval)
{
    spdlog::info("sending messages to {}...", sess.Get().url);

    int num_errors = 0;
    std::vector<float> durations;

    while (running) {
        auto ms = msg(sess, "performance.ping", {});

        if (ms.has_value()) {
            spdlog::get("ping")->info("{:.0f}ms", ms.value());
            durations.push_back(ms.value());
        } else {
            ++num_errors;
        }

        std::this_thread::sleep_for(interval);
    }

    return std::make_tuple(durations, num_errors);
}

cpr::Session login(const std::string& url, const std::string& user, const std::string& password, std::chrono::milliseconds timeout)
{
    spdlog::info("login...");

    cpr::Session sess;
    sess.SetUrl(url + "/cmd.php");
    sess.SetTimeout(timeout);

    auto res = msg(sess, "login.login", {{"login", user}, {"pwd", password}});

    if (!res.has_value()) {
        spdlog::error("login failed");
        std::exit(2);
    }

    return sess;
}

void logout(cpr::Session& sess)
{
    spdlog::info("logout...");

    msg(sess, "login.logout", {});
}

void show_stats(const std::vector<float>& durations, const int num_errors)
{
    fmt::print("messages successful: {}, errors: {}\n", durations.size(), num_errors);
    fmt::print("mean: {:.2f}ms\n", mean(durations));
    fmt::print("median: {:.2f}ms\n", median(durations));
}

void show_usage_and_exit(const clipp::group& cli, const char* argv0)
{
    std::string progname{std::filesystem::path{argv0}.filename().string()};
    fmt::print("{}", clipp::make_man_page(cli, progname).prepend_section("DESCRIPTION", "    Send ping messages."));

    std::exit(1);
}

auto eval_args(int argc, char* argv[])
{
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
        show_usage_and_exit(cli, argv[0]);

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);
    spdlog::info("command line option \"user\": {}", user);
    spdlog::info("command line option \"password\": ???");
    spdlog::info("command line option --log: {}", logfile_name);
    spdlog::info("command line option --interval: {}s", interval);
    spdlog::info("command line option --timeout: {}ms", timeout);

    if (show_help)
        show_usage_and_exit(cli, argv[0]);

    return std::make_tuple(url, user, password, logfile_name, std::chrono::seconds{interval}, std::chrono::milliseconds{timeout});
}

int main(int argc, char* argv[])
{
    const auto [url, user, password, logfile_name, interval, timeout] = eval_args(argc, argv);

    std::signal(SIGINT, signal_handler);
    create_ping_logger(logfile_name);

    auto sess = login(url, user, password, timeout);
    const auto [durations, num_errors] = continuously_send_pings(sess, interval);
    logout(sess);

    show_stats(durations, num_errors);
}
