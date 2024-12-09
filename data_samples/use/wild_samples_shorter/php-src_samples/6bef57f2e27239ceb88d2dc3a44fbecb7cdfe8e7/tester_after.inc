            return new Response(null, true);
        }
        if (is_null($uri)) {
            $uri = $this->makeSourceFile();
        }

        $params = array_merge(
            [
            ],
            $headers
        );
        try {
            $this->response = new Response(
                $this->getClient($address, $connKeepAlive)->request_data($params, false)
            );
        return $filePath;
    }

    /**
     * @return string
     */
    public function makeSourceFile()
    {
        return $this->makeFile('src.php', $this->code);
    }

    /**
     * @param string|null $msg
     */
    private function message($msg)