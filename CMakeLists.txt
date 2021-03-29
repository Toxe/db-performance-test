cmake_minimum_required(VERSION 3.13)

set(VCPKG_FEATURE_FLAGS versions)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("Database Performance Test" LANGUAGES CXX)

# default compiler options and warnings
include(cmake/DefaultCompilerOptionsAndWarnings.cmake)

# static analyzers
include(cmake/StaticAnalyzers.cmake)

# sanitizers
include(cmake/Sanitizers.cmake)

find_package(clipp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Sqlpp11 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

find_library(sqlpp11_mysql_LIBRARY sqlpp-mysql)
find_library(mariadb_LIBRARY mariadb)

add_subdirectory(src)
