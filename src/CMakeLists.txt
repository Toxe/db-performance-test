set(ALL_TARGETS
    db_insert
    http_ping
    msg_ping
)

add_executable(db_insert db_insert.cpp
                         performance.h)
add_executable(http_ping http_ping.cpp
                         combined_logger.cpp combined_logger.h
                         statistics.cpp statistics.h)
add_executable(msg_ping msg_ping.cpp
                        combined_logger.cpp combined_logger.h
                        statistics.cpp statistics.h)

foreach(target ${ALL_TARGETS})
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})
    target_link_libraries(${target} PRIVATE ${SANITIZER_FLAGS})
endforeach()

target_link_libraries(db_insert PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json sqlpp11::sqlpp11 ${sqlpp11_mysql_LIBRARY} libmariadb mariadbclient)
target_link_libraries(http_ping PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json cpr)
target_link_libraries(msg_ping PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json cpr)
