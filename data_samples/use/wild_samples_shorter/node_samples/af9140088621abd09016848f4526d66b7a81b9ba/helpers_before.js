const { getOptionValue } = require('internal/options');
const { setOwnProperty } = require('internal/util');

let debug = require('internal/util/debuglog').debuglog('module', (fn) => {
  debug = fn;
});

            filepath = fileURLToPath(destination);
            urlToFileCache.set(href, filepath);
          }
          return mod.require(filepath);
        }
      }
      if (missing) {
        reaction(new ERR_MANIFEST_DEPENDENCY_MISSING(
          ArrayPrototypeJoin([...conditions], ', '),
        ));
      }
      return mod.require(specifier);
    };
  } else {
    require = function require(path) {
      return mod.require(path);
    };
  }

  function resolve(request, options) {