		if (empty($ignore) || !strpos($a, $ignore)) {
			echo $a;
			$n--;
		}
	}
} /* }}} */

function run_request($host, $port, $uri='/ping', $query='') { /* {{{ */
	require_once 'fcgi.inc';
	$client = new Adoy\FastCGI\Client($host, $port);
	$params = array(
		'GATEWAY_INTERFACE' => 'FastCGI/1.0',
		'REQUEST_METHOD'    => 'GET',
		'SCRIPT_FILENAME'   => $uri,
		'SCRIPT_NAME'       => $uri,
		'QUERY_STRING'      => $query,
		'REQUEST_URI'       => $uri . ($query ? '?'.$query : ""),
		'DOCUMENT_URI'      => $uri,
		'SERVER_SOFTWARE'   => 'php/fcgiclient',
		'REMOTE_ADDR'       => '127.0.0.1',
		'REMOTE_PORT'       => '9985',
		'SERVER_ADDR'       => '127.0.0.1',
		'SERVER_PORT'       => '80',
		'SERVER_NAME'       => php_uname('n'),
		'SERVER_PROTOCOL'   => 'HTTP/1.1',
		'CONTENT_TYPE'      => '',
		'CONTENT_LENGTH'    => 0
	);
	return $client->request($params, false)."\n";
}
/* }}} */
		if (empty($ignore) || !strpos($a, $ignore)) {
			echo $a;
			$n--;
		}
	}
} /* }}} */

function run_request($host, $port, $uri='/ping', $query='') { /* {{{ */
	require_once 'fcgi.inc';
	$client = new Adoy\FastCGI\Client($host, $port);
	$params = array(
		'GATEWAY_INTERFACE' => 'FastCGI/1.0',
		'REQUEST_METHOD'    => 'GET',
		'SCRIPT_FILENAME'   => $uri,
		'SCRIPT_NAME'       => $uri,
		'QUERY_STRING'      => $query,
		'REQUEST_URI'       => $uri . ($query ? '?'.$query : ""),
		'DOCUMENT_URI'      => $uri,
		'SERVER_SOFTWARE'   => 'php/fcgiclient',
		'REMOTE_ADDR'       => '127.0.0.1',
		'REMOTE_PORT'       => '9985',
		'SERVER_ADDR'       => '127.0.0.1',
		'SERVER_PORT'       => '80',
		'SERVER_NAME'       => php_uname('n'),
		'SERVER_PROTOCOL'   => 'HTTP/1.1',
		'CONTENT_TYPE'      => '',
		'CONTENT_LENGTH'    => 0
	);
	return $client->request($params, false)."\n";
}
/* }}} */