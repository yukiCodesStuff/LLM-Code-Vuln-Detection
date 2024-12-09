echo "*** Testing fputcsv() : with default enclosure & delimiter of two chars ***\n";

$fo = new SplFileObject('SplFileObject_fputcsv.csv', 'w');

var_dump($fo->fputcsv(array('water', 'fruit'), ',,', '"'));

unset($fo);

echo "Done\n";
?>
--CLEAN--
<?php
unlink('SplFileObject::fputcsv.csv');
?>
--EXPECTF--
*** Testing fputcsv() : with default enclosure & delimiter of two chars ***

Warning: SplFileObject::fputcsv(): delimiter must be a character in %s on line %d
bool(false)
Done
echo "*** Testing fputcsv() : with default enclosure & delimiter of two chars ***\n";

$fo = new SplFileObject('SplFileObject_fputcsv.csv', 'w');

var_dump($fo->fputcsv(array('water', 'fruit'), ',,', '"'));

unset($fo);

echo "Done\n";
?>
--CLEAN--
<?php
unlink('SplFileObject::fputcsv.csv');
?>
--EXPECTF--
*** Testing fputcsv() : with default enclosure & delimiter of two chars ***

Warning: SplFileObject::fputcsv(): delimiter must be a character in %s on line %d
bool(false)
Done