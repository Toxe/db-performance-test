#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>

#include <clipp.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sqlpp11/mysql/mysql.h>
#include <sqlpp11/sqlpp11.h>

#include "performance.h"

using namespace std::chrono_literals;

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
    int num_insert_rows = 10000;
    int num_rows_per_multi_insert = 1000;
    bool run_single = false;
    bool run_multi = false;
    bool run_all = true;
    bool show_help = false;
    auto log_level = spdlog::level::warn;
    std::string db_config_filename{"mysql.json"};

    auto cli = (
        (clipp::option("--single").set(run_single).set(run_all, false)
            % "run test: single inserts for every row",
         clipp::option("--multi").set(run_multi).set(run_all, false)
            % "run test: insert multiple rows in one request") |
        clipp::option("--all").set(run_all)
            % "run all tests (default)",
        (clipp::option("--config") & clipp::value("filename", db_config_filename))
            % fmt::format("database connection config (default: {})", db_config_filename),
        (clipp::option("--rows") & clipp::value("num_insert_rows", num_insert_rows))
            % fmt::format("number of insert rows (default: {})", num_insert_rows),
        (clipp::option("--rows_per_multi_insert") & clipp::value("num_rows_per_multi_insert", num_rows_per_multi_insert))
            % fmt::format("number of rows per multi insert (default: {})", num_rows_per_multi_insert),
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show verbose output"
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0]);

    spdlog::set_level(log_level);
    spdlog::info("command line option --single: {}", run_single);
    spdlog::info("command line option --multi: {}", run_multi);
    spdlog::info("command line option --all: {}", run_all);
    spdlog::info("command line option --config: {}", db_config_filename);
    spdlog::info("command line option --rows: {}", num_insert_rows);
    spdlog::info("command line option --rows_per_multi_insert: {}", num_rows_per_multi_insert);

    if (run_all) {
        run_single = true;
        run_multi = true;
    }

    if (show_help || !(run_single || run_multi))
        show_usage_and_exit(cli, argv[0]);

    return std::make_tuple(run_single, run_multi, db_config_filename, num_insert_rows, num_rows_per_multi_insert);
}

sqlpp::mysql::connection connect_database(const std::shared_ptr<sqlpp::mysql::connection_config> config)
{
    spdlog::info("connecting to database \"{}\"", config->database);
    return sqlpp::mysql::connection(config);
}

void drop_table(sqlpp::mysql::connection& db, const std::string_view& table_name)
{
    spdlog::info("drop table \"{}\"", table_name);

    db.execute(fmt::format("DROP TABLE IF EXISTS {}", table_name));
}

void create_table(sqlpp::mysql::connection& db, const std::string_view& table_name)
{
    spdlog::info("create table \"{}\"", table_name);

    db.execute(fmt::format(
        "CREATE TABLE {} ("
        "    id     INT NOT NULL AUTO_INCREMENT,"
        "    time   DATETIME NOT NULL,"
        "    text   VARCHAR(255) NOT NULL,"
        "    PRIMARY KEY (id)"
        ") CHARSET=utf8 COLLATE=utf8_unicode_ci",
        table_name));
}

void test_single_inserts(sqlpp::mysql::connection& db, const int num_insert_rows)
{
    spdlog::info("run test: single inserts for every row");
    std::this_thread::sleep_for(1s);

    auto t0 = std::chrono::high_resolution_clock::now();

    Performance::Performance performance{};

    for (int i = 0; i < num_insert_rows; ++i) {
        db(sqlpp::insert_into(performance).set(
            performance.time = std::chrono::system_clock::now(),
            performance.text = fmt::format("single insert, row {}/{}", i+1, num_insert_rows)));
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    fmt::print("test single: {} rows in {} ms\n", num_insert_rows, std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
}

void test_multiple_inserts(sqlpp::mysql::connection& db, const int num_insert_rows, const int num_rows_per_multi_insert)
{
    spdlog::info("run test: insert multiple rows in one request");
    std::this_thread::sleep_for(1s);

    auto t0 = std::chrono::high_resolution_clock::now();

    Performance::Performance performance{};
    auto multi_insert = sqlpp::insert_into(performance).columns(performance.time, performance.text);

    for (int i = 0; i < num_insert_rows; ++i) {
        multi_insert.values.add(
            performance.time = std::chrono::system_clock::now(),
            performance.text = fmt::format("multi insert, row {}/{}", i+1, num_insert_rows));

        if (std::ssize(multi_insert.values._data._insert_values) == num_rows_per_multi_insert) {
            db(multi_insert);
            multi_insert.values._data._insert_values.clear();
        }
    }

    if (!multi_insert.values._data._insert_values.empty())
        db(multi_insert);

    auto t1 = std::chrono::high_resolution_clock::now();

    fmt::print("test multi: {} rows in {} ms\n", num_insert_rows, std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
}

int main(int argc, char* argv[])
{
    auto [run_single, run_multi, db_config_filename, num_insert_rows, num_rows_per_multi_insert] = eval_args(argc, argv);
    auto config = read_mysql_config(db_config_filename);
    auto db = connect_database(config);

    drop_table(db, "performance");
    create_table(db, "performance");

    if (run_single)
        test_single_inserts(db, num_insert_rows);

    if (run_single)
        test_multiple_inserts(db, num_insert_rows, num_rows_per_multi_insert);
}
