--TEST--
Test finfo_file() function : regex rules
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); 
if (substr(PHP_OS, 0, 3) == 'WIN') {
    die('skip.. only for Non Windows Systems');
}