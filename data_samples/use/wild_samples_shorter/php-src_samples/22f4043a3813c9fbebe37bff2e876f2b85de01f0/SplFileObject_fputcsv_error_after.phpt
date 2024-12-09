SplFileObject::fputcsv(): error conditions
--FILE--
<?php
$fo = new SplFileObject(__DIR__ . '/SplFileObject_fputcsv.csv', 'w');

echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fputcsv() with zero argument --\n";
var_dump( $fo->fputcsv($fields, $delim, $enclosure, $fo) );

echo "Done\n";
--CLEAN--
<?php
$file = __DIR__ . '/SplFileObject_fputcsv.csv';
unlink($file);
?>
--EXPECTF--
*** Testing error conditions ***
-- Testing fputcsv() with zero argument --
