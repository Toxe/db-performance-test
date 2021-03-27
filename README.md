# Database Performance Test

Dependencies:

- cmake
- vcpkg
    - spdlog
    - fmt
    - clipp
    - cpr
    - nlohmann-json
    - sqlpp11
    - sqlpp11-connector-mysql


- PHP:
    - perf.ping
    - perf.db_insert_single: table, rows
    - perf.db_insert_multiple: table, rows
    - perf.create_cos: parent, count
- C++:
    - msg_ping
    - msg_db_insert
    - msg_create_cos
    - db_insert
    - html_ping

db_insert --single  
db_insert --multi  
db_insert --all
