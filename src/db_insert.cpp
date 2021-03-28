#include <filesystem>
#include <fstream>
#include <string>
#include <tuple>

#include <clipp.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sqlpp11/mysql/mysql.h>
#include <sqlpp11/sqlpp11.h>

// Read JSON MySQL config file.
//
// Example "mysql.json":
//
//   {
//       "user": "username",
//       "password": "password",
//       "database": "db_performance_test",
//       "unix_socket": "/tmp/mysql.sock"
//   }
std::shared_ptr<sqlpp::mysql::connection_config> read_mysql_config(const char* filename)
{
    std::ifstream in(filename);
    spdlog::info("open database config file: {}", filename);

    if (!in.is_open()) {
        spdlog::error("database config file not found: {}", filename);
        std::exit(2);
    }

    nlohmann::json data;
    in >> data;

    auto config = std::make_shared<sqlpp::mysql::connection_config>();

    if (!data["host"].empty()) config->host = data["host"].get<std::string>();
    if (!data["user"].empty()) config->user = data["user"].get<std::string>();
    if (!data["password"].empty()) config->password = data["password"].get<std::string>();
    if (!data["database"].empty()) config->database = data["database"].get<std::string>();
    if (!data["unix_socket"].empty()) config->unix_socket = data["unix_socket"].get<std::string>();
    if (!data["charset"].empty()) config->charset = data["charset"].get<std::string>();
    if (!data["port"].empty()) config->port = data["port"].get<unsigned int>();
    if (!data["client_flag"].empty()) config->client_flag = data["client_flag"].get<unsigned long>();
    if (!data["auto_reconnect"].empty()) config->auto_reconnect = data["auto_reconnect"].get<bool>();
    if (!data["debug"].empty()) config->debug = data["debug"].get<bool>();

    return config;
}

void show_usage_and_exit(const clipp::group& cli, const char* argv0)
{
    std::string progname{std::filesystem::path{argv0}.filename().string()};
    fmt::print("{}", clipp::make_man_page(cli, progname).prepend_section("DESCRIPTION", "    Run database performance tests."));

    std::exit(1);
}

auto eval_args(int argc, char* argv[])
{
    bool run_single = false;
    bool run_multi = false;
    bool run_all = true;
    bool show_help = false;
    auto log_level = spdlog::level::warn;

    auto cli = (
        (clipp::option("--single").set(run_single).set(run_all, false).doc("run single inserts for every row"),
         clipp::option("--multi").set(run_multi).set(run_all, false).doc("insert multiple rows in one request")) |
        clipp::option("--all").set(run_all).doc("run all tests (default)"),
        clipp::option("-h", "--help").set(show_help).doc("show help"),
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info).doc("show verbose output")
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0]);

    spdlog::set_level(log_level);
    spdlog::info("CLI option --single: {}", run_single);
    spdlog::info("CLI option --multi: {}", run_multi);
    spdlog::info("CLI option --all: {}", run_all);

    if (run_all) {
        run_single = true;
        run_multi = true;
    }

    if (show_help || !(run_single || run_multi))
        show_usage_and_exit(cli, argv[0]);

    return std::make_tuple(run_single, run_multi);
}

int main(int argc, char* argv[])
{
    auto [run_single, run_multi] = eval_args(argc, argv);
    auto config = read_mysql_config("mysql.json");
    sqlpp::mysql::connection db(config);

    spdlog::info("run single test: {}", run_single);
    spdlog::info("run multi test: {}", run_multi);
}
