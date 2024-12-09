$c=str_repeat("B", 65535);
var_dump(chunk_split($a,$b,$c));

$a=str_repeat("B", 65536);
$b=1;
$c=str_repeat("B", 65536);
var_dump(chunk_split($a,$b,$c));


?>
--EXPECT--
a-b-c-
foooo

test|end
bool(false)
bool(false)