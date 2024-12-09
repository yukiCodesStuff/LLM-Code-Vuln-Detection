<?php

$verbose = '...';

if ( isset($_GET["id"]) && isset($_GET["stock"]) && isset($_GET["color"]) ) {
    $id = intval($_GET["id"]);
    $stock = intval($_GET["stock"]);
    $color = $_GET["color"];

    $lst = array(
        $id => $id,
        $stock => $stock,
        $color => $mysqlDB->real_escape_string($color)
    );

    $result = mysqli_query($mysqlDB, "SELECT * FROM `products` WHERE color = '$color' AND (id > $id AND stock > $stock)");

    if ( $result->num_rows > 0) {
        $verbose = htmlentities($result->num_rows . ' exist with color: ' . $color);
    } else {
        $verbose = 'OUT OF STOCK';
    }
}

$mysqlDB->close();

?>

<!DOCTYPE html>
<html>
<form action="/" method="GET">
<label>Pick your color:</label>
<select name="color">
    <option value="white">White</option>
    <option value="red">Red</option>
    <option value="blue">Blue</option>
    <option value="black">Black</option>
    <option value="yellow">Yellow</option>
    <option value="orange">Orange</option>
</select>
<input type="hidden" name="id" value="0">
<input type="hidden" name="stock" value="0">
<input type="submit" value="Search by color">
</form>

<p>Search result: <?= $verbose ?></p>
<body>
</html>
