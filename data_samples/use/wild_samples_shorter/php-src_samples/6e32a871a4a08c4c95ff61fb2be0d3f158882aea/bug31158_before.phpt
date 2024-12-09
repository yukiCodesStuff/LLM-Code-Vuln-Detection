--TEST--
Bug #31158 (array_splice on $GLOBALS crashes)
--FILE--
<?php
function __(){
  $GLOBALS['a'] = "bug\n";