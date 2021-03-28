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
std::shared_ptr<sqlpp::mysql::connection_config> read_mysql_config(const std::string& db_config_filename)
{
    std::ifstream in(db_config_filename);
    spdlog::info("open database config file: {}", db_config_filename);

    if (!in.is_open()) {
        spdlog::error("database config file not found: {}", db_config_filename);
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
    std::string db_config_filename{"mysql.json"};

    auto cli = (
        (clipp::option("--single").set(run_single).set(run_all, false).doc("run single inserts for every row"),
         clipp::option("--multi").set(run_multi).set(run_all, false).doc("insert multiple rows in one request")) |
        clipp::option("--all").set(run_all).doc("run all tests (default)"),
        (clipp::option("--config") & clipp::value("filename", db_config_filename)) % fmt::format("database connection config (default: {})", db_config_filename),
        clipp::option("-h", "--help").set(show_help).doc("show help"),
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info).doc("show verbose output")
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0]);

    spdlog::set_level(log_level);
    spdlog::info("command line option --single: {}", run_single);
    spdlog::info("command line option --multi: {}", run_multi);
    spdlog::info("command line option --all: {}", run_all);
    spdlog::info("command line option --config: {}", db_config_filename);

    if (run_all) {
        run_single = true;
        run_multi = true;
    }

    if (show_help || !(run_single || run_multi))
        show_usage_and_exit(cli, argv[0]);

    return std::make_tuple(run_single, run_multi, db_config_filename);
}

sqlpp::mysql::connection db_connect(const std::shared_ptr<sqlpp::mysql::connection_config> config)
{
    spdlog::info("connecting to database \"{}\"", config->database);
    return sqlpp::mysql::connection(config);
}

int main(int argc, char* argv[])
{
    auto [run_single, run_multi, db_config_filename] = eval_args(argc, argv);
    auto config = read_mysql_config(db_config_filename);
    auto db = db_connect(config);

    spdlog::info("run single test: {}", run_single);
    spdlog::info("run multi test: {}", run_multi);
}
