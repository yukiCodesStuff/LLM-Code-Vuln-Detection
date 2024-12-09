);

$dd = new DOMDocument;
$dd->resolveExternals = true;
$r = $dd->loadXML($xml);
var_dump($dd->validate());
