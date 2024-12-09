--TEST--
Bug #66356 (Heap Overflow Vulnerability in imagecrop())
--SKIPIF--
<?php
	if(!extension_loaded('gd')){ die('skip gd extension not available'); }
?>
--FILE--
<?php
$img = imagecreatetruecolor(10, 10);

// POC #1
var_dump(imagecrop($img, array("x" => "a", "y" => 0, "width" => 10, "height" => 10)));

$arr = array("x" => "a", "y" => "12b", "width" => 10, "height" => 10);
var_dump(imagecrop($img, $arr));
print_r($arr);

// POC #2
var_dump(imagecrop($img, array("x" => 0, "y" => 0, "width" => -1, "height" => 10)));

// POC #3
var_dump(imagecrop($img, array("x" => -20, "y" => -20, "width" => 10, "height" => 10)));

// POC #4
var_dump(imagecrop($img, array("x" => 0x7fffff00, "y" => 0, "width" => 10, "height" => 10)));

?>
--EXPECTF--
resource(%d) of type (gd)
resource(%d) of type (gd)
Array
(
    [x] => a
    [y] => 12b
    [width] => 10
    [height] => 10
)
bool(false)
resource(%d) of type (gd)
resource(%d) of type (gd)
--TEST--
Bug #66356 (Heap Overflow Vulnerability in imagecrop())
--SKIPIF--
<?php
	if(!extension_loaded('gd')){ die('skip gd extension not available'); }
?>
--FILE--
<?php
$img = imagecreatetruecolor(10, 10);

// POC #1
var_dump(imagecrop($img, array("x" => "a", "y" => 0, "width" => 10, "height" => 10)));

$arr = array("x" => "a", "y" => "12b", "width" => 10, "height" => 10);
var_dump(imagecrop($img, $arr));
print_r($arr);

// POC #2
var_dump(imagecrop($img, array("x" => 0, "y" => 0, "width" => -1, "height" => 10)));

// POC #3
var_dump(imagecrop($img, array("x" => -20, "y" => -20, "width" => 10, "height" => 10)));

// POC #4
var_dump(imagecrop($img, array("x" => 0x7fffff00, "y" => 0, "width" => 10, "height" => 10)));

?>
--EXPECTF--
resource(%d) of type (gd)
resource(%d) of type (gd)
Array
(
    [x] => a
    [y] => 12b
    [width] => 10
    [height] => 10
)
bool(false)
resource(%d) of type (gd)
resource(%d) of type (gd)