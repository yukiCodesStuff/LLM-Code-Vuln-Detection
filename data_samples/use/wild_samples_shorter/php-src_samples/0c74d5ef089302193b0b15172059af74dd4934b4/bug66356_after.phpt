// POC #4
var_dump(imagecrop($img, array("x" => 0x7fffff00, "y" => 0, "width" => 10, "height" => 10)));

// bug 66815
var_dump(imagecrop($img, array("x" => 0, "y" => 0, "width" => 65535, "height" => 65535)));
?>
--EXPECTF--
resource(%d) of type (gd)
resource(%d) of type (gd)
    [width] => 10
    [height] => 10
)

Warning: imagecrop(): gd warning: one parameter to a memory allocation multiplication is negative or zero, failing operation gracefully
 in %sbug66356.php on line %d
bool(false)
resource(%d) of type (gd)
resource(%d) of type (gd)

Warning: imagecrop(): gd warning: product of memory allocation multiplication would exceed INT_MAX, failing operation gracefully
 in %sbug66356.php on line %d
bool(false)