from serial import Serial
from sys import exit
';
$finfo = new finfo(FILEINFO_MIME_TYPE);
echo $finfo->buffer($a) . "\n";
$finfo = new finfo();
echo $finfo->buffer($a) . "\n";
?>
--EXPECT--
text/x-script.python
Python script, ASCII text executable