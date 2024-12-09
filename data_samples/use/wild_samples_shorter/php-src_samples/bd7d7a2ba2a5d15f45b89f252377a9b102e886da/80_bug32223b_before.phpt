end;
' LANGUAGE plpgsql;");

function tester() {
        $res = pg_query(dbh, 'SELECT test_notice()');
        $row = pg_fetch_row($res, 0);
		var_dump($row);
?>
===DONE===
--EXPECTF--
array(1) {
  [0]=>
  string(1) "f"
}