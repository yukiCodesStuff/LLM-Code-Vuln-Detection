--TEST--
Bug#59597 NumberFormatter::parse() with TYPE_INT64 results in a 32 bit integer
--SKIPIF--
<?php if( !extension_loaded( 'intl' ) ) print 'skip'; ?>
<?php echo PHP_INT_SIZE != 8 ? "skip 64-bit only" : "OK"; ?>
--FILE--
<?php

$formatter = new \NumberFormatter('en', \NumberFormatter::DECIMAL);
$value = $formatter->parse('2147483647', \NumberFormatter::TYPE_INT32);
var_dump($value);

$formatter = new \NumberFormatter('en', \NumberFormatter::DECIMAL);
$value = $formatter->parse('2147483650', \NumberFormatter::TYPE_INT64);
var_dump($value);

?>
--EXPECTREGEX--
int(2147483647)
int(2147483650)
