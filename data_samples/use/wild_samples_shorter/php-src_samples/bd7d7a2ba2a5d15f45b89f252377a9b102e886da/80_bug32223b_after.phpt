end;
' LANGUAGE plpgsql;");

$res = pg_query($dbh, 'SET client_min_messages TO NOTICE;');
var_dump($res);

function tester() {
        $res = pg_query(dbh, 'SELECT test_notice()');
        $row = pg_fetch_row($res, 0);
		var_dump($row);
?>
===DONE===
--EXPECTF--
resource(%d) of type (pgsql result)
resource(%d) of type (pgsql result)
array(1) {
  [0]=>
  string(1) "f"
}