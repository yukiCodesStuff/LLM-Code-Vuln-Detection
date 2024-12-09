$diff = date_diff($date, $other);

var_dump($diff);
--EXPECT--
object(DateInterval)#3 (8) {
  ["y"]=>
  int(0)
  ["m"]=>
  int(0)
  int(0)
  ["s"]=>
  int(0)
  ["invert"]=>
  int(0)
  ["days"]=>
  int(3)
}