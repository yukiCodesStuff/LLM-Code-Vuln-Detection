  if (parsed != null) {
    // Avoid accessing the `protocol` property due to the lazy getters.
    protocol = parsed.protocol;
    if (protocol === 'data:' ||
      (experimentalNetworkImports &&
        (
          protocol === 'https:' ||
      return { __proto__: null, url: parsed.href };
    }
  }

  // There are multiple deep branches that can either throw or return; instead
  // of duplicating that deeply nested logic for the possible returns, DRY and
  // check for a return. This seems the least gnarly.
  const maybeReturn = checkIfDisallowedImport(