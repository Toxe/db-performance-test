set(ALL_TARGETS
    db_insert
    http_ping
    msg_create_cos
    msg_db_insert
    msg_ping
    convert_log_to_csv
)

add_executable(db_insert db_insert.cpp
                         performance.h
                         common/combined_logger.cpp common/combined_logger.h
                         common/usage.cpp common/usage.h)
add_executable(http_ping http_ping.cpp
                         common/combined_logger.cpp common/combined_logger.h
                         common/statistics.cpp common/statistics.h
                         common/usage.cpp common/usage.h)
add_executable(msg_create_cos msg_create_cos.cpp
                              common/combined_logger.cpp common/combined_logger.h
                              common/msg.cpp common/msg.h
                              common/usage.cpp common/usage.h)
add_executable(msg_db_insert msg_db_insert.cpp
                             common/combined_logger.cpp common/combined_logger.h
                             common/msg.cpp common/msg.h
                             common/usage.cpp common/usage.h)
add_executable(msg_ping msg_ping.cpp
                        common/combined_logger.cpp common/combined_logger.h
                        common/msg.cpp common/msg.h
                        common/statistics.cpp common/statistics.h
                        common/usage.cpp common/usage.h)
add_executable(convert_log_to_csv convert_log_to_csv.cpp
                                  common/usage.cpp common/usage.h)

foreach(target ${ALL_TARGETS})
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})
    target_link_libraries(${target} PRIVATE ${SANITIZER_FLAGS})
endforeach()

target_include_directories(convert_log_to_csv PRIVATE ${pcre2_INCLUDE_DIRS})

target_link_libraries(db_insert PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json sqlpp11::sqlpp11 ${sqlpp11_mysql_LIBRARY} libmariadb mariadbclient)
target_link_libraries(http_ping PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json cpr)
target_link_libraries(msg_create_cos PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json cpr)
target_link_libraries(msg_db_insert PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json cpr)
target_link_libraries(msg_ping PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json cpr)
target_link_libraries(convert_log_to_csv PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only ${pcre2_LIBRARY})
