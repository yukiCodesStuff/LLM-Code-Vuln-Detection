<?php
function PathFilter($s) {
    $s = preg_replace("/(:\/\/)|\\\\/", "", $s);
    while ( str_contains($s, "../") ) {
        $s = str_replace("../", "", $s);
    }
    return $s;
}

$content = 'Missing parameter "file", No file given.';

if ( isset($_GET['file']) && $_GET['file'] != "" ) {
    $file =  htmlspecialchars( PathFilter($_GET['file']) );
    $content = file_get_contents( $file );

    if ( strlen($content) <= 0 ) {
        $content = 'Could not find file: ' . htmlentities($file);
    }
}
?>