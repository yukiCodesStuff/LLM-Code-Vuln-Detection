from serial import Serial
from sys import exit
';
// As of libmagic >= 5.41 libmagic delivers text/plain for this buffer,
// to be observed further
$finfo = new finfo(FILEINFO_MIME_TYPE);
echo $finfo->buffer($a) . "\n";
$finfo = new finfo();
echo $finfo->buffer($a) . "\n";
?>
--EXPECT--
text/plain
a env python script, ASCII text executable