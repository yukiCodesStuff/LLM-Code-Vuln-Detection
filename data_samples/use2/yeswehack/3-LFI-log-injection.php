<?php

function IncludeFilter($str) {
    while (True) {
        if ( strpos($str, "../") == false ) {
            break;
        }
        $str = str_replace("../", "", $str);
    }
    return $str;
}

function OSPath($str){
    if ( strtolower(PHP_OS) == "linux" ) {
        $str = str_replace("\\", "/", $str);
        
    } else {
        $str = str_replace("/", "\\", $str);
    }
    return $str;
}

function Logging($value) {
    file_put_contents("logs/log.txt", (date("[Y-m-d]") . "$value\n"), FILE_APPEND);
}

$lang = ( isset($_GET['lang']) ) ? $_GET['lang'] : "en";

Logging($lang);
include(OSPath("home/" . IncludeFilter($lang)));

?>
