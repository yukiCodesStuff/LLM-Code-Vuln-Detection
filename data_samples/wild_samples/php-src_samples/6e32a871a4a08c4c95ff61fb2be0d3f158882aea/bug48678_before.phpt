--TEST--
Bug #48678 (DateInterval segfaults when unserialising)
--FILE--
<?php
$x = new DateInterval("P3Y6M4DT12H30M5S");
print_r($x);
$y = unserialize(serialize($x));
print_r($y);
--EXPECTF--
DateInterval Object
(
    [y] => 3
    [m] => 6
    [d] => 4
    [h] => 12
    [i] => 30
    [s] => 5
    [invert] => 0
    [days] =>%s
)
DateInterval Object
(
    [y] => 3
    [m] => 6
    [d] => 4
    [h] => 12
    [i] => 30
    [s] => 5
    [invert] => 0
    [days] =>%s
)
--TEST--
Bug #48678 (DateInterval segfaults when unserialising)
--FILE--
<?php
$x = new DateInterval("P3Y6M4DT12H30M5S");
print_r($x);
$y = unserialize(serialize($x));
print_r($y);
--EXPECTF--
DateInterval Object
(
    [y] => 3
    [m] => 6
    [d] => 4
    [h] => 12
    [i] => 30
    [s] => 5
    [invert] => 0
    [days] =>%s
)
DateInterval Object
(
    [y] => 3
    [m] => 6
    [d] => 4
    [h] => 12
    [i] => 30
    [s] => 5
    [invert] => 0
    [days] =>%s
)