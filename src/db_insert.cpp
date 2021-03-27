#include "clipp.h"
#include <filesystem>
#include <iostream>
#include <tuple>

void show_usage_and_exit(const clipp::group& cli, const char* argv0, const char* error_message = nullptr)
{
    if (error_message)
        std::cout << error_message << "\n\n";

    std::string progname{std::filesystem::path{argv0}.filename().string()};
    std::cout << clipp::make_man_page(cli, progname).prepend_section("DESCRIPTION", "    Run database performance tests.") << '\n';

    std::exit(1);
}

auto eval_args(int argc, char* argv[])
{
    bool run_single = false;
    bool run_multi = false;
    bool run_all = true;
    bool show_help = false;

    auto cli = (
        (clipp::option("--single").set(run_single).set(run_all, false).doc("run single inserts for every row"),
         clipp::option("--multi").set(run_multi).set(run_all, false).doc("insert multiple rows in one request")) |
        clipp::option("--all").set(run_all).doc("run all tests (default)"),
        clipp::option("-h", "--help").set(show_help).doc("show help")
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0]);

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

    std::cout << run_single << '\n';
    std::cout << run_multi << '\n';
}
