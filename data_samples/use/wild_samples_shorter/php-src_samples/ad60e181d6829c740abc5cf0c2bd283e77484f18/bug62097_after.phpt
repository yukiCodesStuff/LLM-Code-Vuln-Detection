--TEST--
Bug #62097: fix for bug #54547 is wrong for 32-bit machines
--SKIPIF--
<?php
if (PHP_INT_MAX !== 2147483647)
	die('skip for system with 32-bit wide longs only');
--FILE--
<?php