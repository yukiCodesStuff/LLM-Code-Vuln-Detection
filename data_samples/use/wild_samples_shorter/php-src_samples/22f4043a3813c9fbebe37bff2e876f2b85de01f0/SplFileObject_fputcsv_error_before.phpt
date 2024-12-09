SplFileObject::fputcsv(): error conditions
--FILE--
<?php
$fo = new SplFileObject('SplFileObject_fputcsv.csv', 'w');

echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fputcsv() with zero argument --\n";
var_dump( $fo->fputcsv($fields, $delim, $enclosure, $fo) );

echo "Done\n";
--EXPECTF--
*** Testing error conditions ***
-- Testing fputcsv() with zero argument --
