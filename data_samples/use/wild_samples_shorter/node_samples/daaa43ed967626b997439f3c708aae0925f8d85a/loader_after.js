    redirects = manifest.getDependencyMapper(moduleURL);
    // TODO(rafaelgss): remove the necessity of this branch
    setOwnProperty(this, 'require', makeRequireFunction(this, redirects));
    // eslint-disable-next-line no-proto
    setOwnProperty(this.__proto__, 'require', makeRequireFunction(this, redirects));
  }
  this[require_private_symbol] = internalRequire;
}

  const module = cachedModule || new Module(filename, parent);

  if (isMain) {
    setOwnProperty(process, 'mainModule', module);
    setOwnProperty(module.require, 'main', process.mainModule);
    module.id = '.';
  }
