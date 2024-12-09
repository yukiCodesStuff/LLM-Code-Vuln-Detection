        if ($this->hasError()) {
            return new Response(null, true);
        }
        if (is_null($uri)) {
            $uri = $this->makeFile('src.php', $this->code);
        }

        $params = array_merge(
            [
                'GATEWAY_INTERFACE' => 'FastCGI/1.0',
                'REQUEST_METHOD'    => 'GET',
                'SCRIPT_FILENAME'   => $uri,
                'SCRIPT_NAME'       => $uri,
                'QUERY_STRING'      => $query,
                'REQUEST_URI'       => $uri . ($query ? '?'.$query : ""),
                'DOCUMENT_URI'      => $uri,
                'SERVER_SOFTWARE'   => 'php/fcgiclient',
                'REMOTE_ADDR'       => '127.0.0.1',
                'REMOTE_PORT'       => '7777',
                'SERVER_ADDR'       => '127.0.0.1',
                'SERVER_PORT'       => '80',
                'SERVER_NAME'       => php_uname('n'),
                'SERVER_PROTOCOL'   => 'HTTP/1.1',
                'DOCUMENT_ROOT'     => __DIR__,
                'CONTENT_TYPE'      => '',
                'CONTENT_LENGTH'    => 0
            ],
            $headers
        );

        try {
            $this->response = new Response(
                $this->getClient($address, $connKeepAlive)->request_data($params, false)
            );
            $this->message($successMessage);
        } catch (\Exception $exception) {
            if ($errorMessage === null) {
                $this->error("Request failed", $exception);
            } else {
                $this->message($errorMessage);
            }
            $this->response = new Response();
        }
        if ($this->debug) {
            $this->response->debugOutput();
        }
        return $this->response;
    }
        if (is_null($uri)) {
            $uri = $this->makeFile('src.php', $this->code);
        }

        $params = array_merge(
            [
                'GATEWAY_INTERFACE' => 'FastCGI/1.0',
                'REQUEST_METHOD'    => 'GET',
                'SCRIPT_FILENAME'   => $uri,
                'SCRIPT_NAME'       => $uri,
                'QUERY_STRING'      => $query,
                'REQUEST_URI'       => $uri . ($query ? '?'.$query : ""),
                'DOCUMENT_URI'      => $uri,
                'SERVER_SOFTWARE'   => 'php/fcgiclient',
                'REMOTE_ADDR'       => '127.0.0.1',
                'REMOTE_PORT'       => '7777',
                'SERVER_ADDR'       => '127.0.0.1',
                'SERVER_PORT'       => '80',
                'SERVER_NAME'       => php_uname('n'),
                'SERVER_PROTOCOL'   => 'HTTP/1.1',
                'DOCUMENT_ROOT'     => __DIR__,
                'CONTENT_TYPE'      => '',
                'CONTENT_LENGTH'    => 0
            ],
            $headers
        );

        try {
            $this->response = new Response(
                $this->getClient($address, $connKeepAlive)->request_data($params, false)
            );
            $this->message($successMessage);
        } catch (\Exception $exception) {
            if ($errorMessage === null) {
                $this->error("Request failed", $exception);
            } else {
                $this->message($errorMessage);
            }
            $this->response = new Response();
        }
        if ($this->debug) {
            $this->response->debugOutput();
        }
        return $this->response;
    }

    /**
     * Get client.
     *
     * @param string $address
     * @param bool $keepAlive
     * @return Client
     */
    private function getClient(string $address = null, $keepAlive = false)
    {
        $address = $address ? $this->processTemplate($address) : $this->getAddr();
        if ($address[0] === '/') { // uds
            $host = 'unix://' . $address;
            $port = -1;
        } elseif ($address[0] === '[') { // ipv6
            $addressParts = explode(']:', $address);
            $host = $addressParts[0];
            if (isset($addressParts[1])) {
                $host .= ']';
                $port = $addressParts[1];
            } else {
                $port = $this->getPort();
            }
        } else { // ipv4
            $addressParts = explode(':', $address);
            $host = $addressParts[0];
            $port = $addressParts[1] ?? $this->getPort();
        }

        if (!$keepAlive) {
            return new Client($host, $port);
        }

        if (!isset($this->clients[$host][$port])) {
            $client = new Client($host, $port);
            $client->setKeepAlive(true);
            $this->clients[$host][$port] = $client;
        }

        return $this->clients[$host][$port];
    }

    /**
     * Display logs
     *
     * @param int $number
     * @param string $ignore
     */
    public function displayLog(int $number = 1, string $ignore = 'systemd')
    {
        /* Read $number lines or until EOF */
        while ($number > 0 || ($number < 0 && !feof($this->outDesc))) {
            $a = fgets($this->outDesc);
            if (empty($ignore) || !strpos($a, $ignore)) {
                echo $a;
                $number--;
            }
        }
    }

    /**
     * Get a single log line
     *
     * @return null|string
     */
    private function getLogLine()
    {
        $read = [$this->outDesc];
        $write = null;
        $except = null;
        if (stream_select($read, $write, $except, 2 )) {
            return fgets($this->outDesc);
        } else {
            return null;
        }
    }

    /**
     * Get log lines
     *
     * @param int $number
     * @param bool $skipBlank
     * @param string $ignore
     * @return array
     */
    public function getLogLines(int $number = 1, bool $skipBlank = false, string $ignore = 'systemd')
    {
        $lines = [];
        /* Read $n lines or until EOF */
        while ($number > 0 || ($number < 0 && !feof($this->outDesc))) {
            $line = $this->getLogLine();
            if (is_null($line)) {
                break;
            }
            if ((empty($ignore) || !strpos($line, $ignore)) && (!$skipBlank || strlen(trim($line)) > 0)) {
                $lines[] = $line;
                $number--;
            }
        }

        return $lines;
    }

    /**
     * @return mixed|string
     */
    public function getLastLogLine()
    {
        $lines = $this->getLogLines();

        return $lines[0] ?? '';
    }

    /**
     * Send signal to the supplied PID or the server PID.
     *
     * @param string $signal
     * @param int|null $pid
     * @return string
     */
    public function signal($signal, int $pid = null)
    {
        if (is_null($pid)) {
            $pid = $this->getPid();
        }

        return exec("kill -$signal $pid");
    }

    /**
     * Terminate master process
     */
    public function terminate()
    {
        proc_terminate($this->masterProcess);
    }

    /**
     * Close all open descriptors and process resources
     *
     * @param bool $terminate
     */
    public function close($terminate = false)
    {
        if ($terminate) {
            $this->terminate();
        }
        fclose($this->outDesc);
        proc_close($this->masterProcess);
    }

    /**
     * Create a config file.
     *
     * @param string $extension
     * @return string
     * @throws \Exception
     */
    private function createConfig($extension = 'ini')
    {
        if (is_array($this->configTemplate)) {
            $configTemplates = $this->configTemplate;
            if (!isset($configTemplates['main'])) {
                throw new \Exception('The config template array has to have main config');
            }
            $mainTemplate = $configTemplates['main'];
            unset($configTemplates['main']);
            if (!is_dir(self::CONF_DIR)) {
                mkdir(self::CONF_DIR);
            }
            foreach ($configTemplates as $name => $configTemplate) {
                $this->makeFile(
                    'conf',
                    $this->processTemplate($configTemplate),
                    self::CONF_DIR,
                    $name
                );
            }
        } else {
            $mainTemplate = $this->configTemplate;
        }

        return $this->makeFile($extension, $this->processTemplate($mainTemplate));
    }

    /**
     * Process template string.
     *
     * @param string $template
     * @return string
     */
    private function processTemplate(string $template)
    {
        $vars = [
            'FILE:LOG:ACC' => ['getAbsoluteFile', self::FILE_EXT_LOG_ACC],
            'FILE:LOG:ERR' => ['getAbsoluteFile', self::FILE_EXT_LOG_ERR],
            'FILE:LOG:SLOW' => ['getAbsoluteFile', self::FILE_EXT_LOG_SLOW],
            'FILE:PID' => ['getAbsoluteFile', self::FILE_EXT_PID],
            'RFILE:LOG:ACC' => ['getRelativeFile', self::FILE_EXT_LOG_ACC],
            'RFILE:LOG:ERR' => ['getRelativeFile', self::FILE_EXT_LOG_ERR],
            'RFILE:LOG:SLOW' => ['getRelativeFile', self::FILE_EXT_LOG_SLOW],
            'RFILE:PID' => ['getRelativeFile', self::FILE_EXT_PID],
            'ADDR:IPv4' => ['getAddr', 'ipv4'],
            'ADDR:IPv4:ANY' => ['getAddr', 'ipv4-any'],
            'ADDR:IPv6' => ['getAddr', 'ipv6'],
            'ADDR:IPv6:ANY' => ['getAddr', 'ipv6-any'],
            'ADDR:UDS' => ['getAddr', 'uds'],
            'PORT' => ['getPort', 'ip'],
            'INCLUDE:CONF' => self::CONF_DIR . '/*.conf',
        ];
        $aliases = [
            'ADDR' => 'ADDR:IPv4',
            'FILE:LOG' => 'FILE:LOG:ERR',
        ];
        foreach ($aliases as $aliasName => $aliasValue) {
            $vars[$aliasName] = $vars[$aliasValue];
        }

        return preg_replace_callback(
            '/{{([a-zA-Z0-9:]+)(\[\w+\])?}}/',
            function ($matches) use ($vars) {
                $varName = $matches[1];
                if (!isset($vars[$varName])) {
                    $this->error("Invalid config variable $varName");
                    return 'INVALID';
                }
                $pool = $matches[2] ?? 'default';
                $varValue = $vars[$varName];
                if (is_string($varValue)) {
                    return $varValue;
                }
                $functionName = array_shift($varValue);
                $varValue[] = $pool;
                return call_user_func_array([$this, $functionName], $varValue);
            },
            $template
        );
    }

    /**
     * @param string $type
     * @param string $pool
     * @return string
     */
    public function getAddr(string $type = 'ipv4', $pool = 'default')
    {
        $port = $this->getPort($type, $pool, true);
        if ($type === 'uds') {
            return $this->getFile($port . '.sock');
        }

        return $this->getHost($type) . ':' . $port;
    }

    /**
     * @param string $type
     * @param string $pool
     * @param bool $useAsId
     * @return int
     */
    public function getPort(string $type = 'ip', $pool = 'default', $useAsId = false)
    {
        if ($type === 'uds' && !$useAsId) {
            return -1;
        }

        if (isset($this->ports['values'][$pool])) {
            return $this->ports['values'][$pool];
        }
        $port = ($this->ports['last'] ?? 9000 + PHP_INT_SIZE - 1) + 1;
        $this->ports['values'][$pool] = $this->ports['last'] = $port;

        return $port;
    }

    /**
     * @param string $type
     * @return string
     */
    public function getHost(string $type = 'ipv4')
    {
        switch ($type) {
            case 'ipv6-any':
                return '[::]';
            case 'ipv6':
                return '[::1]';
            case 'ipv4-any':
                return '0.0.0.0';
            default:
                return '127.0.0.1';
        }
    }

    /**
     * Get listen address.
     *
     * @param string|null $template
     * @return string
     */
    public function getListen($template = null)
    {
        return $template ? $this->processTemplate($template) : $this->getAddr();
    }

    /**
     * Get PID.
     *
     * @return int
     */
    public function getPid()
    {
        $pidFile = $this->getFile('pid');
        if (!is_file($pidFile)) {
            return (int) $this->error("PID file has not been created");
        }
        $pidContent = file_get_contents($pidFile);
        if (!is_numeric($pidContent)) {
            return (int) $this->error("PID content '$pidContent' is not integer");
        }

        return (int) $pidContent;
    }


    /**
     * @param string $extension
     * @param string|null $dir
     * @param string|null $name
     * @return string
     */
    private function getFile(string $extension, $dir = null, $name = null)
    {
        $fileName = (is_null($name) ? $this->fileName : $name . '.') . $extension;

        return is_null($dir) ? $fileName : $dir . '/'  . $fileName;
    }

    /**
     * @param string $extension
     * @return string
     */
    private function getAbsoluteFile(string $extension)
    {
        return $this->getFile($extension);
    }

    /**
     * @param string $extension
     * @return string
     */
    private function getRelativeFile(string $extension)
    {
        $fileName = rtrim(basename($this->fileName), '.');

        return $this->getFile($extension, null, $fileName);
    }

    /**
     * @param string $extension
     * @param string $prefix
     * @return string
     */
    private function getPrefixedFile(string $extension, string $prefix = null)
    {
        $fileName = rtrim($this->fileName, '.');
        if (!is_null($prefix)) {
            $fileName = $prefix . '/' . basename($fileName);
        }

        return $this->getFile($extension, null, $fileName);
    }

    /**
     * @param string $extension
     * @param string $content
     * @param string|null $dir
     * @param string|null $name
     * @return string
     */
    private function makeFile(string $extension, string $content = '', $dir = null, $name = null)
    {
        $filePath = $this->getFile($extension, $dir, $name);
        file_put_contents($filePath, $content);

        return $filePath;
    }

    /**
     * @param string|null $msg
     */
    private function message($msg)
    {
        if ($msg !== null) {
            echo "$msg\n";
        }
    }

    /**
     * @param string $msg
     * @param \Exception|null $exception
     */
    private function error($msg, \Exception $exception = null)
    {
        $this->error =  'ERROR: ' . $msg;
        if ($exception) {
            $this->error .= '; EXCEPTION: ' . $exception->getMessage();
        }
        $this->error .= "\n";

        echo $this->error;
    }

    /**
     * @return bool
     */
    private function hasError()
    {
        return !is_null($this->error) || !is_null($this->logTool->getError());
    }

    /**
     * Expect file with a supplied extension to exist.
     *
     * @param string $extension
     * @param string $prefix
     * @return bool
     */
    public function expectFile(string $extension, $prefix = null)
    {
        $filePath = $this->getPrefixedFile($extension, $prefix);
        if (!file_exists($filePath)) {
            return $this->error("The file $filePath does not exist");
        }

        return true;
    }

    /**
     * Expect file with a supplied extension to not exist.
     *
     * @param string $extension
     * @param string $prefix
     * @return bool
     */
    public function expectNoFile(string $extension, $prefix = null)
    {
        $filePath = $this->getPrefixedFile($extension, $prefix);
        if (file_exists($filePath)) {
            return $this->error("The file $filePath exists");
        }

        return true;
    }

    /**
     * Expect message to be written to FastCGI error stream.
     *
     * @param string $message
     * @param int $limit
     * @param int $repeat
     */
    public function expectFastCGIErrorMessage(
        string $message,
        int $limit = 1024,
        int $repeat = 0
    ) {
        $this->logTool->setExpectedMessage($message, $limit, $repeat);
        $this->logTool->checkTruncatedMessage($this->response->getErrorData());
    }

    /**
     * Expect starting lines to be logged.
     */
    public function expectLogStartNotices()
    {
        $this->logTool->expectStartingLines($this->getLogLines(2));
    }

    /**
     * Expect terminating lines to be logged.
     */
    public function expectLogTerminatingNotices()
    {
        $this->logTool->expectTerminatorLines($this->getLogLines(-1));
    }

    /**
     * Expect log message that can span multiple lines.
     *
     * @param string $message
     * @param int $limit
     * @param int $repeat
     * @param bool $decorated
     * @param bool $wrapped
     */
    public function expectLogMessage(
        string $message,
        int $limit = 1024,
        int $repeat = 0,
        bool $decorated = true,
        bool $wrapped = true
    ) {
        $this->logTool->setExpectedMessage($message, $limit, $repeat);
        if ($wrapped) {
            $logLines = $this->getLogLines(-1, true);
            $this->logTool->checkWrappedMessage($logLines, true, $decorated);
        } else {
            $logLines = $this->getLogLines(1, true);
            $this->logTool->checkTruncatedMessage($logLines[0] ?? '');
        }
        if ($this->debug) {
            $this->message("-------------- LOG LINES: -------------");
            var_dump($logLines);
            $this->message("---------------------------------------\n");
        }
    }

    /**
     * Expect a single log line.
     *
     * @param string $message
     * @return bool
     */
    public function expectLogLine(string $message, bool $is_stderr = true)
    {
        $messageLen = strlen($message);
        $limit = $messageLen > 1024 ? $messageLen + 16 : 1024;
        $this->logTool->setExpectedMessage($message, $limit);
        $logLines = $this->getLogLines(1, true);
        if ($this->debug) {
            $this->message("LOG LINE: " . ($logLines[0] ?? ''));
        }

        return $this->logTool->checkWrappedMessage($logLines, false, true, $is_stderr);
    }

    /**
     * Expect a log debug message.
     *
     * @param string $message
     * @param string|null $pool
     * @return bool
     */
    public function expectLogDebug(string $message, $pool = null)
    {
        return $this->logTool->expectDebug($this->getLastLogLine(), $message, $pool);
    }

    /**
     * Expect a log notice.
     *
     * @param string $message
     * @param string|null $pool
     * @return bool
     */
    public function expectLogNotice(string $message, $pool = null)
    {
        return $this->logTool->expectNotice($this->getLastLogLine(), $message, $pool);
    }

    /**
     * Expect a log warning.
     *
     * @param string $message
     * @param string|null $pool
     * @return bool
     */
    public function expectLogWarning(string $message, $pool = null)
    {
        return $this->logTool->expectWarning($this->getLastLogLine(), $message, $pool);
    }

    /**
     * Expect a log error.
     *
     * @param string $message
     * @param string|null $pool
     * @return bool
     */
    public function expectLogError(string $message, $pool = null)
    {
        return $this->logTool->expectError($this->getLastLogLine(), $message, $pool);
    }

    /**
     * Expect a log alert.
     *
     * @param string $message
     * @param string|null $pool
     * @return bool
     */
    public function expectLogAlert(string $message, $pool = null)
    {
        return $this->logTool->expectAlert($this->getLastLogLine(), $message, $pool);
    }

    /**
     * Expect no log lines to be logged.
     *
     * @return bool
     */
    public function expectNoLogMessages()
    {
        $logLines = $this->getLogLines(-1, true);
        if (!empty($logLines)) {
            return $this->error(
                "Expected no log lines but following lines logged:\n" . implode("\n", $logLines)
            );
        }

        return true;
    }

    /**
     * Print content of access log.
     */
    public function printAccessLog()
    {
        $accessLog = $this->getFile('acc.log');
        if (is_file($accessLog)) {
            print file_get_contents($accessLog);
        }
    }
}
    {
        $filePath = $this->getFile($extension, $dir, $name);
        file_put_contents($filePath, $content);

        return $filePath;
    }

    /**
     * @param string|null $msg
     */
    private function message($msg)
    {
        if ($msg !== null) {
            echo "$msg\n";
        }