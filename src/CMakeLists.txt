set(ALL_TARGETS
    db_insert
)

add_executable(db_insert db_insert.cpp include/performance.h)

foreach(target ${ALL_TARGETS})
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})
    target_link_libraries(${target} PRIVATE ${SANITIZER_FLAGS})
endforeach()

target_include_directories(db_insert PRIVATE include)
target_link_libraries(db_insert PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only nlohmann_json::nlohmann_json sqlpp11::sqlpp11 ${sqlpp11_mysql_LIBRARY} libmariadb mariadbclient)
