--TEST--
Bug #53437 (Crash when using unserialized DatePeriod instance), variation 1
--FILE--
<?php
$dp = new DatePeriod(new DateTime('2010-01-01 UTC'), new DateInterval('P1D'), 2);

var_dump($dpu);

echo "Unserialized:\r\n";
foreach($dpu as $dt) {
        echo $dt->format('Y-m-d H:i:s')."\r\n";
}
?>
==DONE==
--EXPECT--
Original:
2010-01-01 00:00:00
2010-01-02 00:00:00
2010-01-03 00:00:00

object(DatePeriod)#1 (6) {
  ["start"]=>
  object(DateTime)#2 (3) {
    ["date"]=>
    string(19) "2010-01-01 00:00:00"
    ["timezone_type"]=>
    int(3)
    ["timezone"]=>
    string(3) "UTC"
  }
  ["current"]=>
  object(DateTime)#4 (3) {
    ["date"]=>
    string(19) "2010-01-04 00:00:00"
    ["timezone_type"]=>
    int(3)
    ["timezone"]=>
    string(3) "UTC"
  }
  ["end"]=>
  NULL
  ["interval"]=>
  object(DateInterval)#5 (15) {
    ["y"]=>
    int(0)
    ["m"]=>
    int(0)
    ["d"]=>
    int(1)
    ["h"]=>
    int(0)
    ["i"]=>
    int(0)
    ["s"]=>
    int(0)
    ["weekday"]=>
    int(0)
    ["weekday_behavior"]=>
    int(0)
    ["first_last_day_of"]=>
    int(0)
    ["invert"]=>
    int(0)
    ["days"]=>
    bool(false)
    ["special_type"]=>
    int(0)
    ["special_amount"]=>
    int(0)
    ["have_weekday_relative"]=>
    int(0)
    ["have_special_relative"]=>
    int(0)
  }
  ["recurrences"]=>
  int(3)
  ["include_start_date"]=>
  bool(true)
}
object(DatePeriod)#5 (6) {
  ["start"]=>
  object(DateTime)#10 (3) {
    ["date"]=>
    string(19) "2010-01-01 00:00:00"
    ["timezone_type"]=>
    int(3)
    ["timezone"]=>
    string(3) "UTC"
  }
  ["current"]=>
  object(DateTime)#7 (3) {
    ["date"]=>
    string(19) "2010-01-04 00:00:00"
    ["timezone_type"]=>
    int(3)
    ["timezone"]=>
    string(3) "UTC"
  }
  ["end"]=>
  NULL
  ["interval"]=>
  object(DateInterval)#8 (15) {
    ["y"]=>
    int(0)
    ["m"]=>
    int(0)
    ["d"]=>
    int(1)
    ["h"]=>
    int(0)
    ["i"]=>
    int(0)
    ["s"]=>
    int(0)
    ["weekday"]=>
    int(0)
    ["weekday_behavior"]=>
    int(0)
    ["first_last_day_of"]=>
    int(0)
    ["invert"]=>
    int(0)
    ["days"]=>
    int(0)
    ["special_type"]=>
    int(0)
    ["special_amount"]=>
    int(0)
    ["have_weekday_relative"]=>
    int(0)
    ["have_special_relative"]=>
    int(0)
  }
  ["recurrences"]=>
  int(3)
  ["include_start_date"]=>
  bool(true)
}
Unserialized:
2010-01-01 00:00:00
2010-01-02 00:00:00
2010-01-03 00:00:00
==DONE==