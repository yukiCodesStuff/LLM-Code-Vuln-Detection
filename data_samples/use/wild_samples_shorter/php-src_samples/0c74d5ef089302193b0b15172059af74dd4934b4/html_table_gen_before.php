
ksort($multicp_rows);
foreach ($multicp_rows as &$v) { ksort($v); }

echo
"/* {{{ Start of double code point tables for $name */", "\n\n";
