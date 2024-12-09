SplFileObject::fputcsv(): Checking data after calling the function
--FILE--
<?php
$fo = new SplFileObject('SplFileObject_fputcsv.csv', 'w');

$data = array(1, 2, 'foo', 'haha', array(4, 5, 6), 1.3, null);

$fo->fputcsv($data);
?>
--CLEAN--
<?php
unlink('SplFileObject::fputcsv.csv');
?>
--EXPECTF--
Notice: Array to string conversion in %s on line %d
array(7) {