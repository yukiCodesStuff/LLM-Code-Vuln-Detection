    };
  } else {
    require = function require(path) {
      return mod[require_private_symbol](mod, path);
    };
  }

  function resolve(request, options) {