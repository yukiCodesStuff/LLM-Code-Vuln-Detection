echo "*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***\n";

$fo = new SplFileObject('SplFileObject_fputcsv.csv', 'w');

var_dump($fo->fputcsv(array('water', 'fruit'), ',,', '""'));

unset($fo);

echo "Done\n";
?>
--CLEAN--
<?php
unlink('SplFileObject::fputcsv.csv');
?>
--EXPECTF--
*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***

Warning: SplFileObject::fputcsv(): enclosure must be a character in %s on line %d
bool(false)
Done
echo "*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***\n";

$fo = new SplFileObject('SplFileObject_fputcsv.csv', 'w');

var_dump($fo->fputcsv(array('water', 'fruit'), ',,', '""'));

unset($fo);

echo "Done\n";
?>
--CLEAN--
<?php
unlink('SplFileObject::fputcsv.csv');
?>
--EXPECTF--
*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***

Warning: SplFileObject::fputcsv(): enclosure must be a character in %s on line %d
bool(false)
Done