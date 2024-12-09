  if (parsed != null) {
    // Avoid accessing the `protocol` property due to the lazy getters.
    protocol = parsed.protocol;
    if (protocol === 'data:' ||
      (experimentalNetworkImports &&
        (
          protocol === 'https:' ||
          protocol === 'http:'
        )
      )
    ) {
      return { __proto__: null, url: parsed.href };
    }
    ) {
      return { __proto__: null, url: parsed.href };
    }
  }

  // There are multiple deep branches that can either throw or return; instead
  // of duplicating that deeply nested logic for the possible returns, DRY and
  // check for a return. This seems the least gnarly.
  const maybeReturn = checkIfDisallowedImport(
    specifier,
    parsed,
    parsedParentURL,
  );

  if (maybeReturn) { return maybeReturn; }

  // This must come after checkIfDisallowedImport
  protocol ??= parsed?.protocol;
  if (protocol === 'node:') { return { __proto__: null, url: specifier }; }


  const isMain = parentURL === undefined;
  if (isMain) {
    parentURL = getCWDURL().href;

    // This is the initial entry point to the program, and --input-type has
    // been passed as an option; but --input-type can only be used with
    // --eval, --print or STDIN string input. It is not allowed with file
    // input, to avoid user confusion over how expansive the effect of the
    // flag should be (i.e. entry point only, package scope surrounding the
    // entry point, etc.).
    if (inputTypeFlag) { throw new ERR_INPUT_TYPE_NOT_ALLOWED(); }
  }

  conditions = getConditionsSet(conditions);
  let url;
  try {
    url = moduleResolve(
      specifier,
      parentURL,
      conditions,
      isMain ? preserveSymlinksMain : preserveSymlinks,
    );
  } catch (error) {
    // Try to give the user a hint of what would have been the
    // resolved CommonJS module
    if (error.code === 'ERR_MODULE_NOT_FOUND' ||
        error.code === 'ERR_UNSUPPORTED_DIR_IMPORT') {
      if (StringPrototypeStartsWith(specifier, 'file://')) {
        specifier = fileURLToPath(specifier);
      }
      decorateErrorWithCommonJSHints(error, specifier, parentURL);
    }
    throw error;
  }

  return {
    __proto__: null,
    // Do NOT cast `url` to a string: that will work even when there are real
    // problems, silencing them
    url: url.href,
    format: defaultGetFormatWithoutErrors(url, context),
  };
}

/**
 * Decorates the given error with a hint for CommonJS modules.
 * @param {Error} error - The error to decorate.
 * @param {string} specifier - The specifier that was attempted to be imported.
 * @param {string} parentURL - The URL of the parent module.
 */
function decorateErrorWithCommonJSHints(error, specifier, parentURL) {
  const found = resolveAsCommonJS(specifier, parentURL);
  if (found && found !== specifier) { // Don't suggest the same input the user provided.
    // Modify the stack and message string to include the hint
    const endOfFirstLine = StringPrototypeIndexOf(error.stack, '\n');
    const hint = `Did you mean to import ${JSONStringify(found)}?`;
    error.stack =
      StringPrototypeSlice(error.stack, 0, endOfFirstLine) + '\n' +
      hint +
      StringPrototypeSlice(error.stack, endOfFirstLine);
    error.message += `\n${hint}`;
  }
}

module.exports = {
  decorateErrorWithCommonJSHints,
  defaultResolve,
  encodedSepRegEx,
  packageExportsResolve,
  packageImportsResolve,
  throwIfInvalidParentURL,
  legacyMainResolve,
};

// cycle
const {
  defaultGetFormatWithoutErrors,
} = require('internal/modules/esm/get_format');