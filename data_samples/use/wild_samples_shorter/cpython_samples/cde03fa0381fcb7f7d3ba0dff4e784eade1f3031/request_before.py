        name = name.lower()
        if value and name[-6:] == '_proxy':
            proxies[name[:-6]] = value
    for name, value in os.environ.items():
        if name[-6:] == '_proxy':
            name = name.lower()
            if value: