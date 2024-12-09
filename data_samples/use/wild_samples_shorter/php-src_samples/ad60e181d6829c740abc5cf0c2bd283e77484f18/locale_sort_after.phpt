  die("skip setlocale() failed\n");
}
?>
--FILE--
<?php
setlocale(LC_ALL, 'fr_FR.ISO8859-1', 'fr_FR');
$table = array("AB" => "Alberta",