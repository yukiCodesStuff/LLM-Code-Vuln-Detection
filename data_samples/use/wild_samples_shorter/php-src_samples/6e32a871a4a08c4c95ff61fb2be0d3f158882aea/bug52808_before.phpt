echo "==DONE==\n";
?>
--EXPECTF--
object(DateInterval)#%d (8) {
  ["y"]=>
  int(1)
  ["m"]=>
  int(2)
  int(30)
  ["s"]=>
  int(0)
  ["invert"]=>
  int(1)
  ["days"]=>
  int(437)
}
object(DateInterval)#%d (8) {
  ["y"]=>
  int(0)
  ["m"]=>
  int(9)
  int(30)
  ["s"]=>
  int(0)
  ["invert"]=>
  int(0)
  ["days"]=>
  int(294)
}
object(DateInterval)#%d (8) {
  ["y"]=>
  int(0)
  ["m"]=>
  int(9)
  int(30)
  ["s"]=>
  int(0)
  ["invert"]=>
  int(0)
  ["days"]=>
  int(294)
}
DateInterval::__construct(): Failed to parse interval (2007-05-11T15:30:00Z/)
DateInterval::__construct(): Failed to parse interval (2007-05-11T15:30:00Z)
DateInterval::__construct(): Unknown or bad format (2007-05-11T15:30:00Z/:00Z)