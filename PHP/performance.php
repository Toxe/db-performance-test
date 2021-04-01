<?php
// Package: performance

// performance.ping {{{
MSG("performance.ping", array(
    "in"    => array(),
    "out"   => array(),
    "check" => array("default_msg_check"),
    "doc"   => "Macht nichts weiter als Null zurückzugeben."
));

function msg_performance_ping($in, &$out)
{
}
// }}}

// performance.db_insert_single {{{
MSG("performance.db_insert_single", array(
    "in"    => array("rows" => PARAM_INT),
    "out"   => array("duration"),
    "check" => array("dev"),
    "doc"   => "Datensätze in einzelnen INSERTs hinzufügen."
));

function msg_performance_db_insert_single($in, &$out)
{
    $num_insert_rows = (int) $in["rows"];

    if ($num_insert_rows < 1)
        return -1;

    performance_drop_table("performance");
    performance_create_table("performance");
    $duration = performance_test_single_inserts("performance", $num_insert_rows);

    if ($duration === FALSE)
        return -2;

    $out["duration"] = $duration;
}
// }}}

// performance.db_insert_multi {{{
MSG("performance.db_insert_multi", array(
    "in"    => array("rows"                  => PARAM_INT,
                     "rows_per_multi_insert" => PARAM_INT),
    "out"   => array("duration"),
    "check" => array("dev"),
    "doc"   => "Mehrere Datensätze in einzelnen INSERTs zusammenfassen."
));

function msg_performance_db_insert_multi($in, &$out)
{
    $num_insert_rows = (int) $in["rows"];
    $num_rows_per_multi_insert = (int) $in["rows_per_multi_insert"];

    if ($num_insert_rows < 1 || $num_rows_per_multi_insert < 1)
        return -1;

    performance_drop_table("performance");
    performance_create_table("performance");
    $duration = performance_test_multiple_inserts("performance", $num_insert_rows, $num_rows_per_multi_insert);

    if ($duration === FALSE)
        return -2;

    $out["duration"] = $duration;
}
// }}}

// performance.create_cos {{{
MSG("performance.create_cos", array(
    "in"    => array("count"  => PARAM_INT),
    "out"   => array("duration"),
    "check" => array("dev"),
    "doc"   => "COs unterhalb eines Users anlegen."
));

function msg_performance_create_cos($in, &$out)
{
    PACKAGE("co_comment");
    PACKAGE("co_settings");

    $count = (int) $in["count"];

    if ($count < 1)
        return -1;

    $co_parent = CURRENT_USER();

    if (!$co_parent)
        return -2;

    performance_delete_children($co_parent);
    $duration = performance_create_children($co_parent, $count);

    if ($duration === FALSE)
        return -3;

    $out["duration"] = $duration;
}
// }}}

// ---------------------------------------------------------------------------

function performance_drop_table($table_name)
{
    global $core;

    return $core->db->query("DROP TABLE IF EXISTS `{$table_name}`");
}

function performance_create_table($table_name)
{
    global $core;

    $sql = "CREATE TABLE `{$table_name}` (" .
           "    id     INT NOT NULL AUTO_INCREMENT," .
           "    time   DATETIME NOT NULL," .
           "    text   VARCHAR(255) NOT NULL," .
           "    PRIMARY KEY (id)" .
           ") CHARSET=utf8 COLLATE=utf8_unicode_ci";

    return $core->db->query($sql);
}

function performance_test_single_inserts($table_name, $num_insert_rows)
{
    global $core;

    sleep(1);
    $t_start = microtime(TRUE);

    for ($i = 1; $i <= $num_insert_rows; ++$i) {
        $t = strftime("%Y-%m-%d %H:%M:%S", time());
        $s = "single insert, row {$i}/{$num_insert_rows}";
        $sql = "INSERT INTO `{$table_name}` (time, text) VALUES ('$t', '$s')";

        if (!$core->db->query($sql))
            return FALSE;
    }

    return (int) ((microtime(TRUE) - $t_start) * 1000.0);
}

function performance_test_multiple_inserts($table_name, $num_insert_rows, $num_rows_per_multi_insert)
{
    global $core;

    sleep(1);
    $t_start = microtime(TRUE);

    $insert_values = [];

    for ($i = 1; $i <= $num_insert_rows; ++$i) {
        $t = strftime("%Y-%m-%d %H:%M:%S", time());
        $s = "multi insert, row {$i}/{$num_insert_rows}";
        $insert_values[] = "('$t', '$s')";

        if (count($insert_values) == $num_rows_per_multi_insert) {
            $sql = "INSERT INTO `{$table_name}` (time, text) VALUES ". implode(",", $insert_values);
            $insert_values = [];

            if (!$core->db->query($sql))
                return FALSE;
        }
    }

    if (count($insert_values) > 0) {
        $sql = "INSERT INTO `{$table_name}` (time, text) VALUES ". implode(",", $insert_values);

        if (!$core->db->query($sql))
            return FALSE;
    }

    return (int) ((microtime(TRUE) - $t_start) * 1000.0);
}

function performance_delete_children($co_parent)
{
    $co_settings = settings_find_unique($co_parent, "performance.create_cos");

    if ($co_settings)
        $co_settings->delete();
}

function performance_create_children($co_parent, $count)
{
    global $core;

    sleep(1);
    $t_start = microtime(TRUE);

    $co_settings = settings_create($co_parent, "performance.create_cos", [
        "time" => strftime("%Y-%m-%d %H:%M:%S", time()),
        "count" => $count
    ]);

    if (!$co_settings)
        return FAlSE;

    for ($i = 1; $i <= $count; ++$i) {
        $co_comment = comment_add($co_settings, "create_cos", "CO {$i}/{$count}");

        if (!$co_comment)
            return FALSE;
    }

    return (int) ((microtime(TRUE) - $t_start) * 1000.0);
}

// vim:et:ts=2:sw=2:foldmethod=marker:
