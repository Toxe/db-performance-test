cmake_minimum_required(VERSION 3.13)
project("Database Performance Testzers" LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# default compiler options and warnings
include(cmake/DefaultCompilerOptionsAndWarnings.cmake)

# static analyzers
include(cmake/StaticAnalyzers.cmake)

# sanitizers
include(cmake/Sanitizers.cmake)

find_package(clipp CONFIG REQUIRED)

add_subdirectory(src)
