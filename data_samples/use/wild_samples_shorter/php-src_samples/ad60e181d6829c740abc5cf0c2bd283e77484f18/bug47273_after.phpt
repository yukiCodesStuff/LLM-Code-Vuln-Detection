Bug #47273 (Encoding bug in SoapServer->fault)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
$request1 = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>