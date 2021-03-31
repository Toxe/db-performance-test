# Database Performance Test

Tools for testing database and server performance.

## Dependencies:

- CMake
- Vcpkg
    - clipp
    - cpr
    - fmt
    - nlohmann-json
    - spdlog
    - sqlpp11
    - sqlpp11-connector-mysql
    - libmariadb

## Usage

### db_insert

```
DESCRIPTION
    Run database performance tests.

SYNOPSIS
        db_insert [([--single] [--multi]) | --all] [--config <filename>] [--rows <num_insert_rows>]
                  [--rows_per_multi_insert <num_rows_per_multi_insert>] [-h] [-v]

OPTIONS
        --single    run test: single inserts for every row
        --multi     run test: insert multiple rows in one request
        --all       run all tests (default)
        --config <filename>
                    database connection config (default: mysql.json)

        --rows <num_insert_rows>
                    number of insert rows (default: 10000)

        --rows_per_multi_insert <num_rows_per_multi_insert>
                    number of rows per multi insert (default: 1000)

        -h, --help  show help
        -v, --verbose
                    show verbose output

EXAMPLE
    $ db_insert --rows 1000 --rows_per_multi_insert 100 --config ../mysql.json
```

### http_ping

```
DESCRIPTION
    Ping a URL.

SYNOPSIS
        http_ping [-h] [-v] <host> [--log <logfile>] [--interval <interval>] [--timeout <timeout>]

OPTIONS
        -h, --help  show help
        -v, --verbose
                    show verbose output

        <host>      URL to ping
        --log <logfile>
                    logfile name (default: logs/http_ping.log)

        --interval <interval>
                    wait "interval" seconds between each request (default: 1s)

        --timeout <timeout>
                    request timeout in milliseconds (default: 30000ms)

EXAMPLE
    $ http_ping https://example.com
```

### msg_ping

```
DESCRIPTION
    Send ping messages.

SYNOPSIS
        msg_ping [-h] [-v] <host> <user> <password> [--log <logfile>] [--interval <interval>]
                 [--timeout <timeout>]

OPTIONS
        -h, --help  show help
        -v, --verbose
                    show verbose output

        <host>      Host URL
        <user>      Login user name
        <password>  Login password
        --log <logfile>
                    logfile name (default: logs/msg_ping.log)

        --interval <interval>
                    wait "interval" seconds between each request (default: 1s)

        --timeout <timeout>
                    request timeout in milliseconds (default: 30000ms)

EXAMPLE
    $ msg_ping https://example.com user password
```
