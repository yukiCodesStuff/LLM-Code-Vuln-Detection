);

$dd = new DOMDocument;
$dd->substituteEntities = true;
$dd->resolveExternals = true;
$r = $dd->loadXML($xml);
var_dump($dd->validate());
