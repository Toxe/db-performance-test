#define PCRE2_CODE_UNIT_WIDTH 8

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include <clipp.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <pcre2.h>
#include <spdlog/spdlog.h>

#include "common/usage.h"

void convert_file(std::ifstream& in, std::ofstream& out, pcre2_code* re, pcre2_match_context* mcontext, pcre2_match_data* match_data)
{
    std::string line;

    out << "\"time\",\"url\",\"ms\"\n";

    while (std::getline(in, line)) {
        const PCRE2_SPTR subject = reinterpret_cast<PCRE2_SPTR>(line.c_str());
        const int rc = pcre2_jit_match(re, subject, line.size(), 0, 0, match_data, mcontext);

        if (rc == 3+1) {
            const PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);

            for (int i = 1; i < rc; ++i) {
                const PCRE2_SPTR substring_start = subject + ovector[2 * i];
                const int substring_length = static_cast<int>(ovector[2 * i + 1] - ovector[2 * i]);
                const std::string_view s{reinterpret_cast<const char*>(substring_start), static_cast<std::string_view::size_type>(substring_length)};

                out << '"' << s << '"';

                if (i < 3)
                    out << ',';
            }

            out << '\n';
        }
    }
}

std::tuple<pcre2_code*, pcre2_match_context*, pcre2_jit_stack*, pcre2_match_data*> init_pcre2(const char* pattern)
{
    int errorcode;
    PCRE2_SIZE erroroffset;

    pcre2_code* re = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(pattern), PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroroffset, nullptr);

    if (!re)
        throw std::runtime_error{"PCRE2 compilation failed"};

    if (pcre2_jit_compile(re, PCRE2_JIT_COMPLETE) < 0)
        throw std::runtime_error{"PCRE2 JIT compile error"};

    pcre2_match_context* mcontext = pcre2_match_context_create(nullptr);

    if (!mcontext)
        throw std::runtime_error{"PCRE2 unable to create match context"};

    pcre2_jit_stack* jit_stack = pcre2_jit_stack_create(32*1024, 512*1024, nullptr);

    if (!jit_stack)
        throw std::runtime_error{"PCRE2 unable to create JIT stack"};

    pcre2_jit_stack_assign(mcontext, nullptr, jit_stack);

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, nullptr);

    if (!match_data)
        throw std::runtime_error{"PCRE2 unable to create match data"};

    return {re, mcontext, jit_stack, match_data};
}

void cleanup_pcre2(pcre2_code* re, pcre2_match_context* mcontext, pcre2_jit_stack* jit_stack, pcre2_match_data* match_data)
{
    pcre2_code_free(re);
    pcre2_match_data_free(match_data);
    pcre2_match_context_free(mcontext);
    pcre2_jit_stack_free(jit_stack);
}

auto eval_args(int argc, char* argv[])
{
    const auto description = "Convert log file to CSV.";
    const auto example = "logs/http_ping.log http_ping.csv";
    bool show_help = false;
    auto log_level = spdlog::level::warn;
    std::string logfile_name;
    std::string csvfile_name;

    auto cli = (
        clipp::option("-h", "--help").set(show_help)
            % "show help",
        clipp::option("-v", "--verbose").set(log_level, spdlog::level::info)
            % "show verbose output",
        clipp::value("logfile_name", logfile_name)
            % "log file name",
        clipp::value("csvfile_name", csvfile_name)
            % "CSV file name"
    );

    if (!clipp::parse(argc, argv, cli))
        show_usage_and_exit(cli, argv[0], description, example);

    spdlog::set_level(log_level);
    spdlog::info("command line option \"logfile_name\": {}", logfile_name);
    spdlog::info("command line option \"csvfile_name\": {}", csvfile_name);

    if (show_help)
        show_usage_and_exit(cli, argv[0], description, example);

    return std::make_tuple(logfile_name, csvfile_name);
}

int main(int argc, char* argv[])
{
    const auto [logfile_name, csvfile_name] = eval_args(argc, argv);

    auto [re, mcontext, jit_stack, match_data] = init_pcre2(R"(\[([^]]+)\] \[info\] ([^ ]+) --> (\d+)ms)");

    std::ifstream in{logfile_name};
    std::ofstream out{csvfile_name};

    convert_file(in, out, re, mcontext, match_data);
    cleanup_pcre2(re, mcontext, jit_stack, match_data);
}
