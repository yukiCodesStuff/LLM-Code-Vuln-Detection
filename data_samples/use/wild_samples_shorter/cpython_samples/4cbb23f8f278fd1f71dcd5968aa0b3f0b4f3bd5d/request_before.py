        name = name.lower()
        if value and name[-6:] == '_proxy':
            proxies[name[:-6]] = value
    return proxies

def proxy_bypass_environment(host):
    """Test if proxies should not be used for a particular host.