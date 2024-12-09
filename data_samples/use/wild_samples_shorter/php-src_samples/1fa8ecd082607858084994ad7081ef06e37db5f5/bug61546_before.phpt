Bug #61546 (functions related to current script failed when chdir() in cli sapi)
--FILE--
<?php
$php = getenv("TEST_PHP_EXECUTABLE");
$test_code = <<<PHP
<?php
chdir('..');
var_dump(get_current_user() != "");
chdir('..');
var_dump(getmyinode() != false);
var_dump(getlastmod() != false);
PHP;

file_put_contents("bug61546_sub.php", $test_code);