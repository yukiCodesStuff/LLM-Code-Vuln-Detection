	if (key_exists($el[1], $multicp_rows)) {
		if (count($el) == 3)
			$multicp_rows[$el[1]][$el[2]] = $el[0];
		else
			$multicp_rows[$el[1]]["default"] = $el[0];
	}
}

if ($pass2 < 2)
    echo "/* {{{ Start of $name multi-stage table for codepoint -> entity */", "\n\n";
else
    echo "/* {{{ Start of $name table for codepoint -> entity */", "\n\n";

if (empty($multicp_rows))
    goto skip_multicp;

ksort($multicp_rows);
foreach ($multicp_rows as &$v) { ksort($v); }

echo
"/* {{{ Start of double code point tables for $name */", "\n\n";

foreach ($multicp_rows as $k => $v) {
	echo "static const entity_multicodepoint_row multi_cp_{$ident}_",
		sprintf("%05s", $k), "[] = {", "\n";
	if (key_exists("default", $v)) {
        if ($v['default'] == 'GT') /* hack to make > translate to &gt; not GT; */
            $v['default'] = "gt";
		echo "\t{ {", sprintf("%02d", count($v) - 1),
			",\t\t", sprintf("\"%-21s", $v["default"].'",'), "\t",
            sprintf("% 2d", strlen($v["default"])), '} },', "\n"; 
	} else {
		echo "\t{ {", sprintf("%02d", count($v)),
			",\t\t", sprintf("%-22s", 'NULL'), ",\t0} },\n"; 
	}
	unset($v["default"]);
	foreach ($v as $l => $w) {
		echo "\t{ {", sprintf("0x%05s", $l), ",\t", sprintf("\"%-21s", $w.'",'), "\t",
            sprintf("% 2d", strlen($w)), '} },', "\n"; 
	}
	echo "};\n";
}