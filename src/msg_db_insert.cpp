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

void test_single_inserts(cpr::Session& sess, const int num_insert_rows)
{
    spdlog::info("run test: single inserts for every row");

    const auto res = msg(sess, "performance.db_insert_single",
        {{"rows", std::to_string(num_insert_rows)}});

    if (res.has_value() && res->status == 0)
        spdlog::get("combined")->info("{}ms", res->json["duration"]);
}

void test_multiple_inserts(cpr::Session& sess, const int num_insert_rows, const int num_rows_per_multi_insert)
{
    spdlog::info("run test: insert multiple rows in one request");

    const auto res = msg(sess, "performance.db_insert_multi",
        {{"rows", std::to_string(num_insert_rows)}, {"rows_per_multi_insert", std::to_string(num_rows_per_multi_insert)}});

    if (res.has_value() && res->status == 0)
        spdlog::get("combined")->info("{}ms", res->json["duration"]);
}

auto eval_args(int argc, char* argv[])
{
    const auto description = "Send messages to run database performance tests.";
    const auto example = "https://example.com user password --rows 1000 --rows_per_multi_insert 100";
    bool show_help = false;
    auto log_level = spdlog::level::warn;
    bool run_single = false;
    bool run_multi = false;
    bool run_all = true;
    int num_insert_rows = 10000;
    int num_rows_per_multi_insert = 1000;
    int timeout = 30000;
    std::string url;
    std::string user;
    std::string password;
    std::string logfile_name{"logs/msg_db_insert.log"};

    auto cli = (
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show verbose output",
        (clipp::option("--single").set(run_single).set(run_all, false)
            % "run test: single inserts for every row",
         clipp::option("--multi").set(run_multi).set(run_all, false)
            % "run test: insert multiple rows in one request") |
        clipp::option("--all").set(run_all)
            % "run all tests (default)",
        clipp::value("host", url)
            % "Host URL",
        clipp::value("user", user)
            % "Login user name",
        clipp::value("password", password)
            % "Login password",
        (clipp::option("--log") & clipp::value("logfile", logfile_name))
            % fmt::format("logfile name (default: {})", logfile_name),
        (clipp::option("--timeout") & clipp::integer("timeout", timeout))
            % fmt::format("request timeout in milliseconds (default: {}ms)", timeout),
        (clipp::option("--rows") & clipp::value("num_insert_rows", num_insert_rows))
            % fmt::format("number of insert rows (default: {})", num_insert_rows),
        (clipp::option("--rows_per_multi_insert") & clipp::value("num_rows_per_multi_insert", num_rows_per_multi_insert))
            % fmt::format("number of rows per multi insert (default: {})", num_rows_per_multi_insert)
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0], description, example);

    spdlog::set_level(log_level);
    spdlog::info("command line option \"url\": {}", url);
    spdlog::info("command line option \"user\": {}", user);
    spdlog::info("command line option \"password\": ???");
    spdlog::info("command line option --log: {}", logfile_name);
    spdlog::info("command line option --timeout: {}ms", timeout);
    spdlog::info("command line option --single: {}", run_single);
    spdlog::info("command line option --multi: {}", run_multi);
    spdlog::info("command line option --all: {}", run_all);
    spdlog::info("command line option --rows: {}", num_insert_rows);
    spdlog::info("command line option --rows_per_multi_insert: {}", num_rows_per_multi_insert);

    if (run_all) {
        run_single = true;
        run_multi = true;
    }

    if (show_help)
        show_usage_and_exit(cli, argv[0], description, example);

    return std::make_tuple(url, user, password, logfile_name, std::chrono::milliseconds{timeout}, run_single, run_multi, num_insert_rows, num_rows_per_multi_insert);
}

int main(int argc, char* argv[])
{
    const auto [url, user, password, logfile_name, timeout, run_single, run_multi, num_insert_rows, num_rows_per_multi_insert] = eval_args(argc, argv);

    create_combined_logger(logfile_name);

    auto sess = msg_login(url, user, password, timeout);

    if (run_single)
        test_single_inserts(sess, num_insert_rows);

    if (run_single)
        test_multiple_inserts(sess, num_insert_rows, num_rows_per_multi_insert);

    msg_logout(sess);
}
