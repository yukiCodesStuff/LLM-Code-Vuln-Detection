--TEST--
PostgreSQL notice function
--SKIPIF--
<?php

include("skipif.inc");

_skip_lc_messages();

?>
--INI--
pgsql.log_notice=1
pgsql.ignore_notice=0
--FILE--
<?php
include 'config.inc';
include 'lcmess.inc';

$db = pg_connect($conn_str);

_set_lc_messages();

pg_query($db, "BEGIN;");
pg_query($db, "BEGIN;");

$msg = pg_last_notice($db);
if ($msg === FALSE) {
	echo "Cannot find notice message in hash\n";
	var_dump($msg);
}