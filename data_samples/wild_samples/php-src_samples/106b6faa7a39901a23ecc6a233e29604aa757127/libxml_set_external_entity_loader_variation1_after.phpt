	function ($public, $system, $context) use($dtd,$entity){
		static $first = true;
		var_dump($public);
		var_dump($system);
		var_dump($context);
		$f = fopen("php://temp", "r+");
		fwrite($f, $first ? $dtd : $entity);
		$first = false;
		rewind($f);
		return $f;
	}
);

$dd = new DOMDocument;
$dd->substituteEntities = true;
$dd->resolveExternals = true;
$r = $dd->loadXML($xml);
var_dump($dd->validate());

echo "Done.\n";

--EXPECTF--
string(10) "-//FOO/BAR"
string(25) "http://example.com/foobar"
array(4) {
  ["directory"]=>
  string(%d) "%s"
  ["intSubName"]=>
  string(3) "foo"
  ["extSubURI"]=>
  string(25) "http://example.com/foobar"
  ["extSubSystem"]=>
  string(10) "-//FOO/BAR"
}