#include <filesystem>
#include <tuple>

#include <clipp.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sqlpp11/mysql/mysql.h>
#include <sqlpp11/sqlpp11.h>


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

    spdlog::info("run single test: {}", run_single);
    spdlog::info("run multi test: {}", run_multi);
}
