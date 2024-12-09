// POC #4
var_dump(imagecrop($img, array("x" => 0x7fffff00, "y" => 0, "width" => 10, "height" => 10)));

?>
--EXPECTF--
resource(%d) of type (gd)
resource(%d) of type (gd)
    [width] => 10
    [height] => 10
)
bool(false)
resource(%d) of type (gd)
resource(%d) of type (gd)