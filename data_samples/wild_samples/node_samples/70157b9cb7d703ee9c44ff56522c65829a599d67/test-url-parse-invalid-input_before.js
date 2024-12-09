              (e) => {
                // The error should be a URIError.
                if (!(e instanceof URIError))
                  return false;

                // The error should be from the JS engine and not from Node.js.
                // JS engine errors do not have the `code` property.
                return e.code === undefined;
              });