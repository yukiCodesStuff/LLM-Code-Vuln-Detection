--TEST--
Bug #53437 (Crash when using unserialized DatePeriod instance)
--XFAIL--
Bug #53437 Not fixed yet
--FILE--
<?php
$dp = new DatePeriod(new DateTime('2010-01-01 UTC'), new DateInterval('P1D'), 2);

var_dump($dpu);

echo "Unserialized:\r\n";
// ???which leads to CRASH:
foreach($dpu as $dt) {
        echo $dt->format('Y-m-d H:i:s')."\r\n";
}
?>
--EXPECT--