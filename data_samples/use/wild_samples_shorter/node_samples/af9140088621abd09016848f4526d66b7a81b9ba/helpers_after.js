const { getOptionValue } = require('internal/options');
const { setOwnProperty } = require('internal/util');

const {
  privateSymbols: {
    require_private_symbol,
  },
} = internalBinding('util');

let debug = require('internal/util/debuglog').debuglog('module', (fn) => {
  debug = fn;
});

            filepath = fileURLToPath(destination);
            urlToFileCache.set(href, filepath);
          }
          return mod[require_private_symbol](mod, filepath);
        }
      }
      if (missing) {
        reaction(new ERR_MANIFEST_DEPENDENCY_MISSING(
          ArrayPrototypeJoin([...conditions], ', '),
        ));
      }
      return mod[require_private_symbol](mod, specifier);
    };
  } else {
    require = function require(path) {
      return mod[require_private_symbol](mod, path);
    };
  }

  function resolve(request, options) {