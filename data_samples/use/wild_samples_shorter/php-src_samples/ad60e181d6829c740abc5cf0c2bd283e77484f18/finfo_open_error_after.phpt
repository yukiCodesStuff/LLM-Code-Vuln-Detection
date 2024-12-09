--TEST--
Test finfo_open() function : error functionality 
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); 
if(substr(PHP_OS, 0, 3) == 'WIN' )
  die("skip Not Valid for Windows");
?>
--FILE--
<?php
/* Prototype  : resource finfo_open([int options [, string arg]])
 * Description: Create a new fileinfo resource. 