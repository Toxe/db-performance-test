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
                  [--rows_per_multi_insert <num_rows_per_multi_insert>] [--log <logfile>] [-h] [-v]

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

        --log <logfile>
                    logfile name (default: logs/db_insert.log)

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

### msg_db_insert

```
DESCRIPTION
    Send messages to run database performance tests.

SYNOPSIS
        msg_db_insert [-h] [-v] [([--single] [--multi]) | --all] <host> <user> <password> [--log
                      <logfile>] [--timeout <timeout>] [--rows <num_insert_rows>]
                      [--rows_per_multi_insert <num_rows_per_multi_insert>]

OPTIONS
        -h, --help  show help
        -v, --verbose
                    show verbose output

        --single    run test: single inserts for every row
        --multi     run test: insert multiple rows in one request
        --all       run all tests (default)
        <host>      Host URL
        <user>      Login user name
        <password>  Login password
        --log <logfile>
                    logfile name (default: logs/msg_db_insert.log)

        --timeout <timeout>
                    request timeout in milliseconds (default: 30000ms)

        --rows <num_insert_rows>
                    number of insert rows (default: 10000)

        --rows_per_multi_insert <num_rows_per_multi_insert>
                    number of rows per multi insert (default: 1000)

EXAMPLE
    $ msg_db_insert https://example.com user password --rows 1000 --rows_per_multi_insert 100
```

### msg_create_cos

```
DESCRIPTION
    Send message to run CO creation test.

SYNOPSIS
        msg_create_cos [-h] [-v] <host> <user> <password> [--log <logfile>] [--timeout <timeout>]
                       [--count <count>]

OPTIONS
        -h, --help  show help
        -v, --verbose
                    show verbose output

        <host>      Host URL
        <user>      Login user name
        <password>  Login password
        --log <logfile>
                    logfile name (default: logs/msg_create_cos.log)

        --timeout <timeout>
                    request timeout in milliseconds (default: 30000ms)

        --count <count>
                    number of COs to create (default: 10)

EXAMPLE
    $ msg_create_cos https://example.com user password
```
