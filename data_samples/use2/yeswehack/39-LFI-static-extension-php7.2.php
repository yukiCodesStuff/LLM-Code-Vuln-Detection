<?php
if ( isset($_GET['page']) ) {
    include($_SERVER['DOCUMENT_ROOT'] . '/views/' . $_GET['page'] . '.php');
} else {
    echo '<h1>I want a page!</h1>';
}
?>

<html>
<head>
    <title>snippet</title>
    <link rel="stylesheet" href="./assets/css/styles.css">
</head>
<body>
<ul class="navbar">
    <a href="./?page=home">Home</a>
    <a href="./?page=about">About</a>
    <a href="./?page=contact">Contact</a>
</ul>
<body>
</html>
