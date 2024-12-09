    for name, value in os.environ.items():
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

    Checks the environment for a variable named no_proxy, which should
    be a list of DNS suffixes separated by commas, or '*' for all hosts.
    """
    no_proxy = os.environ.get('no_proxy', '') or os.environ.get('NO_PROXY', '')
    # '*' is special case for always bypass
    if no_proxy == '*':
        return 1
    # strip port off host
    hostonly, port = splitport(host)
    # check if the host ends with any of the DNS suffixes
    no_proxy_list = [proxy.strip() for proxy in no_proxy.split(',')]
    for name in no_proxy_list:
        if name and (hostonly.endswith(name) or host.endswith(name)):
            return 1
    # otherwise, don't bypass
    return 0


# This code tests an OSX specific data structure but is testable on all
# platforms
def _proxy_bypass_macosx_sysconf(host, proxy_settings):
    """
    Return True iff this host shouldn't be accessed using a proxy

    This function uses the MacOSX framework SystemConfiguration
    to fetch the proxy information.

    proxy_settings come from _scproxy._get_proxy_settings or get mocked ie:
    { 'exclude_simple': bool,
      'exceptions': ['foo.bar', '*.bar.com', '127.0.0.1', '10.1', '10.0/16']
    }
    """
    from fnmatch import fnmatch

    hostonly, port = splitport(host)

    def ip2num(ipAddr):
        parts = ipAddr.split('.')
        parts = list(map(int, parts))
        if len(parts) != 4:
            parts = (parts + [0, 0, 0, 0])[:4]
        return (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3]

    # Check for simple host names:
    if '.' not in host:
        if proxy_settings['exclude_simple']:
            return True

    hostIP = None

    for value in proxy_settings.get('exceptions', ()):
        # Items in the list are strings like these: *.local, 169.254/16
        if not value: continue

        m = re.match(r"(\d+(?:\.\d+)*)(/\d+)?", value)
        if m is not None:
            if hostIP is None:
                try:
                    hostIP = socket.gethostbyname(hostonly)
                    hostIP = ip2num(hostIP)
                except socket.error:
                    continue

            base = ip2num(m.group(1))
            mask = m.group(2)
            if mask is None:
                mask = 8 * (m.group(1).count('.') + 1)
            else:
                mask = int(mask[1:])
            mask = 32 - mask

            if (hostIP >> mask) == (base >> mask):
                return True

        elif fnmatch(host, value):
            return True

    return False


if sys.platform == 'darwin':
    from _scproxy import _get_proxy_settings, _get_proxies

    def proxy_bypass_macosx_sysconf(host):
        proxy_settings = _get_proxy_settings()
        return _proxy_bypass_macosx_sysconf(host, proxy_settings)

    def getproxies_macosx_sysconf():
        """Return a dictionary of scheme -> proxy server URL mappings.

        This function uses the MacOSX framework SystemConfiguration
        to fetch the proxy information.
        """
        return _get_proxies()



    def proxy_bypass(host):
        if getproxies_environment():
            return proxy_bypass_environment(host)
        else:
            return proxy_bypass_macosx_sysconf(host)

    def getproxies():
        return getproxies_environment() or getproxies_macosx_sysconf()


elif os.name == 'nt':
    def getproxies_registry():
        """Return a dictionary of scheme -> proxy server URL mappings.

        Win32 uses the registry to store proxies.

        """
        proxies = {}
        try:
            import winreg
        except ImportError:
            # Std module, so should be around - but you never know!
            return proxies
        try:
            internetSettings = winreg.OpenKey(winreg.HKEY_CURRENT_USER,
                r'Software\Microsoft\Windows\CurrentVersion\Internet Settings')
            proxyEnable = winreg.QueryValueEx(internetSettings,
                                               'ProxyEnable')[0]
            if proxyEnable:
                # Returned as Unicode but problems if not converted to ASCII
                proxyServer = str(winreg.QueryValueEx(internetSettings,
                                                       'ProxyServer')[0])
                if '=' in proxyServer:
                    # Per-protocol settings
                    for p in proxyServer.split(';'):
                        protocol, address = p.split('=', 1)
                        # See if address has a type:// prefix
                        if not re.match('^([^/:]+)://', address):
                            address = '%s://%s' % (protocol, address)
                        proxies[protocol] = address
                else:
                    # Use one setting for all protocols
                    if proxyServer[:5] == 'http:':
                        proxies['http'] = proxyServer
                    else:
                        proxies['http'] = 'http://%s' % proxyServer
                        proxies['https'] = 'https://%s' % proxyServer
                        proxies['ftp'] = 'ftp://%s' % proxyServer
            internetSettings.Close()
        except (WindowsError, ValueError, TypeError):
            # Either registry key not found etc, or the value in an
            # unexpected format.
            # proxies already set up to be empty so nothing to do
            pass
        return proxies

    def getproxies():
        """Return a dictionary of scheme -> proxy server URL mappings.

        Returns settings gathered from the environment, if specified,
        or the registry.

        """
        return getproxies_environment() or getproxies_registry()

    def proxy_bypass_registry(host):
        try:
            import winreg
        except ImportError:
            # Std modules, so should be around - but you never know!
            return 0
        try:
            internetSettings = winreg.OpenKey(winreg.HKEY_CURRENT_USER,
                r'Software\Microsoft\Windows\CurrentVersion\Internet Settings')
            proxyEnable = winreg.QueryValueEx(internetSettings,
                                               'ProxyEnable')[0]
            proxyOverride = str(winreg.QueryValueEx(internetSettings,
                                                     'ProxyOverride')[0])
            # ^^^^ Returned as Unicode but problems if not converted to ASCII
        except WindowsError:
            return 0
        if not proxyEnable or not proxyOverride:
            return 0
        # try to make a host list from name and IP address.
        rawHost, port = splitport(host)
        host = [rawHost]
        try:
            addr = socket.gethostbyname(rawHost)
            if addr != rawHost:
                host.append(addr)
        except socket.error:
            pass
        try:
            fqdn = socket.getfqdn(rawHost)
            if fqdn != rawHost:
                host.append(fqdn)
        except socket.error:
            pass
        # make a check value list from the registry entry: replace the
        # '<local>' string by the localhost entry and the corresponding
        # canonical entry.
        proxyOverride = proxyOverride.split(';')
        # now check if we match one of the registry values.
        for test in proxyOverride:
            if test == '<local>':
                if '.' not in rawHost:
                    return 1
            test = test.replace(".", r"\.")     # mask dots
            test = test.replace("*", r".*")     # change glob sequence
            test = test.replace("?", r".")      # change glob char
            for val in host:
                if re.match(test, val, re.I):
                    return 1
        return 0

    def proxy_bypass(host):
        """Return a dictionary of scheme -> proxy server URL mappings.

        Returns settings gathered from the environment, if specified,
        or the registry.

        """
        if getproxies_environment():
            return proxy_bypass_environment(host)
        else:
            return proxy_bypass_registry(host)

else:
    # By default use environment variables
    getproxies = getproxies_environment
    proxy_bypass = proxy_bypass_environment