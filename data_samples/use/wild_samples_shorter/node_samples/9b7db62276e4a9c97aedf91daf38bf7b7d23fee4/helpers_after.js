    };
  } else {
    require = function require(path) {
      // When no policy manifest, the original prototype.require is sustained
      return mod.require(path);
    };
  }

  function resolve(request, options) {