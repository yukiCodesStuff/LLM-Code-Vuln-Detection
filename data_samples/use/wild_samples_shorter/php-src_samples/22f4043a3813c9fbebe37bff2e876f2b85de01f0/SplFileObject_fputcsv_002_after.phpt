SplFileObject::fputcsv(): Checking data after calling the function
--FILE--
<?php
$fo = new SplFileObject(__DIR__ . '/SplFileObject_fputcsv.csv', 'w');

$data = array(1, 2, 'foo', 'haha', array(4, 5, 6), 1.3, null);

$fo->fputcsv($data);
?>
--CLEAN--
<?php
$file = __DIR__ . '/SplFileObject_fputcsv.csv';
unlink($file);
?>
--EXPECTF--
Notice: Array to string conversion in %s on line %d
array(7) {