--TEST--
Bug #31158 (array_splice on $GLOBALS crashes)
--INI--
error_reporting = E_ALL
--FILE--
<?php
function __(){
  $GLOBALS['a'] = "bug\n";