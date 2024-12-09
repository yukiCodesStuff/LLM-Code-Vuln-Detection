	}
}

function run_request($host, $port, $uri='/ping', $query='', $headers=array()) {
	require_once 'fcgi.inc';
	$client = new Adoy\FastCGI\Client($host, $port);
	$params = array_merge(array(
		'GATEWAY_INTERFACE' => 'FastCGI/1.0',
		'REQUEST_METHOD'    => 'GET',
		'SCRIPT_FILENAME'   => $uri,
		'SCRIPT_NAME'       => $uri,
		'SERVER_PROTOCOL'   => 'HTTP/1.1',
		'CONTENT_TYPE'      => '',
		'CONTENT_LENGTH'    => 0
	), $headers);
	return $client->request($params, false)."\n";
}