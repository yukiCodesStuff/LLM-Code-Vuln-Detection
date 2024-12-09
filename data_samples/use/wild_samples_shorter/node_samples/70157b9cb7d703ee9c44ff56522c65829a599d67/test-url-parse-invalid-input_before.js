                // JS engine errors do not have the `code` property.
                return e.code === undefined;
              });