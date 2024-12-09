
echo "*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***\n";

$fo = new SplFileObject('SplFileObject_fputcsv.csv', 'w');

var_dump($fo->fputcsv(array('water', 'fruit'), ',,', '""'));

unset($fo);
?>
--CLEAN--
<?php
unlink('SplFileObject::fputcsv.csv');
?>
--EXPECTF--
*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***
