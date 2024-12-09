        name = name.lower()
        if value and name[-6:] == '_proxy':
            proxies[name[:-6]] = value

    # CVE-2016-1000110 - If we are running as CGI script, forget HTTP_PROXY
    # (non-all-lowercase) as it may be set from the web server by a "Proxy:"
    # header from the client
    if 'REQUEST_METHOD' in os.environ:
        proxies.pop('http', None)

    return proxies

def proxy_bypass_environment(host):
    """Test if proxies should not be used for a particular host.