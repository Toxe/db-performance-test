#include <chrono>
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
#include "common/usage.h"

using namespace std::chrono_literals;

void msg_create_cos(cpr::Session& sess, const int parent, const int count)
{
    spdlog::info("sending message to {}...", sess.Get().url);

    const auto res = msg(sess, "performance.create_cos", {{"parent", std::to_string(parent)}, {"count", std::to_string(count)}});

    if (res.has_value() && res->status == 0)
        spdlog::get("combined")->info("{}ms", res->json["duration"]);
}

auto eval_args(int argc, char* argv[])
{
    bool show_help = false;
    auto log_level = spdlog::level::warn;
    int timeout = 30000;
    int parent = 0;
    int count = 10;
    std::string url;
    std::string user;
    std::string password;
    std::string logfile_name{"logs/msg_create_cos.log"};

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
        clipp::value("parent", parent)
            % "COID parent",
        (clipp::option("--log") & clipp::value("logfile", logfile_name))
            % fmt::format("logfile name (default: {})", logfile_name),
        (clipp::option("--timeout") & clipp::integer("timeout", timeout))
            % fmt::format("request timeout in milliseconds (default: {}ms)", timeout),
        (clipp::option("--count") & clipp::integer("count", count))
            % fmt::format("number of COs to create (default: {})", count)
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0], "Send message to run CO creation test.", "https://example.com user password");

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);
    spdlog::info("command line option \"user\": {}", user);
    spdlog::info("command line option \"password\": ???");
    spdlog::info("command line option \"parent\": ", parent);
    spdlog::info("command line option --log: {}", logfile_name);
    spdlog::info("command line option --timeout: {}ms", timeout);
    spdlog::info("command line option --count: {}", count);

    if (show_help)
        show_usage_and_exit(cli, argv[0], "Send message to run CO creation test.", "https://example.com user password");

    return std::make_tuple(url, user, password, logfile_name, std::chrono::milliseconds{timeout}, parent, count);
}

int main(int argc, char* argv[])
{
    const auto [url, user, password, logfile_name, timeout, parent, count] = eval_args(argc, argv);

    create_combined_logger(logfile_name);

    auto sess = msg_login(url, user, password, timeout);
    msg_create_cos(sess, parent, count);
    msg_logout(sess);
}
