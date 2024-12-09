<?php

$query = ( isset($_GET['q']) ) ? $_GET['q'] : '';

$lst_blacklist = array("\"", "`", "'", "\\" );

foreach ( $lst_blacklist as $char ) {
  if ( str_contains($query, $char) ) {
     $query = str_replace($char, ("\\".$char), $query);
  }
}

$result = $mysqlDB->query("SELECT * FROM `products` WHERE category = '$query'");

if ( $result->num_rows > 0 ) {
    while($row = $result->fetch_assoc()) {
      echo "Id: " . $row["id"] . "\n",
      "Stock: " . $row["stock"] . "\n",
      "Category: " .$row["category"] . "\n",
      "Color: " .$row["color"] . "\n";
    }
} else {
  echo "0 results for $query";
}

$mysqlDB->close();
?>
