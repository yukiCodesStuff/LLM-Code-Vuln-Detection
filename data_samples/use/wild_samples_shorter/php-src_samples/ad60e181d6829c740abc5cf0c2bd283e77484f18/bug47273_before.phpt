Bug #47273 (Encoding bug in SoapServer->fault)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--INI--
unicode.script_encoding=ISO-8859-1
unicode.output_encoding=ISO-8859-1
--FILE--
<?php
$request1 = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>