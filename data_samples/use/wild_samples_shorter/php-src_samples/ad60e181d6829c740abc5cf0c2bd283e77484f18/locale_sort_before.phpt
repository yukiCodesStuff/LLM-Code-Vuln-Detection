  die("skip setlocale() failed\n");
}
?>
--INI--
unicode.script_encoding=ISO8859-1
unicode.output_encoding=ISO8859-1
--FILE--
<?php
setlocale(LC_ALL, 'fr_FR.ISO8859-1', 'fr_FR');
$table = array("AB" => "Alberta",