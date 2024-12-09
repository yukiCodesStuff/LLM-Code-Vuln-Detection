end;
' LANGUAGE plpgsql;");


$res = pg_query($dbh, 'SELECT test_notice()');
var_dump($res);
$row = pg_fetch_row($res, 0);
var_dump($row);
===DONE===
--EXPECTF--
resource(%d) of type (pgsql result)
array(1) {
  [0]=>
  string(1) "f"
}