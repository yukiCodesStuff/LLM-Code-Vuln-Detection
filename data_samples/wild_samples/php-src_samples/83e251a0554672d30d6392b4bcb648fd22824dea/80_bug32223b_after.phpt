--TEST--
Bug #32223 (weird behaviour of pg_last_notice using define)
--SKIPIF--
<?php 
require_once('skipif.inc');

_skip_lc_messages();
	
@pg_query($conn, "CREATE LANGUAGE 'plpgsql' HANDLER plpgsql_call_handler LANCOMPILER 'PL/pgSQL'");
$res = @pg_query($conn, "CREATE OR REPLACE FUNCTION test_notice() RETURNS boolean AS '
begin
        RAISE NOTICE ''11111'';
        return ''f'';
end;
' LANGUAGE plpgsql;");
if (!$res) die('skip PLPGSQL not available');
?>
--INI--
pgsql.ignore_notice=0
--FILE--
<?php

require_once('config.inc');
require_once('lcmess.inc');

define('dbh', pg_connect($conn_str));
if (!dbh) {
        die ("Could not connect to the server");
}