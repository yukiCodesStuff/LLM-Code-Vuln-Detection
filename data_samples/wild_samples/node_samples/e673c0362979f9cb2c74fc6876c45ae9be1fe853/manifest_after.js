const {
  ArrayIsArray,
  ArrayPrototypeSort,
  ObjectEntries,
  ObjectFreeze,
  ObjectKeys,
  ObjectSetPrototypeOf,
  RegExpPrototypeExec,
  SafeMap,
  SafeSet,
  RegExpPrototypeSymbolReplace,
  StringPrototypeEndsWith,
  StringPrototypeStartsWith,
  Symbol,
} = primordials;
const {
  ERR_MANIFEST_ASSERT_INTEGRITY,
  ERR_MANIFEST_INVALID_RESOURCE_FIELD,
  ERR_MANIFEST_INVALID_SPECIFIER,
  ERR_MANIFEST_UNKNOWN_ONERROR,
} = require('internal/errors').codes;
let debug = require('internal/util/debuglog').debuglog('policy', (fn) => {
  debug = fn;
});
const SRI = require('internal/policy/sri');
const { URL } = require('internal/url');
const { internalVerifyIntegrity } = internalBinding('crypto');
const kRelativeURLStringPattern = /^\.{0,2}\//;
const { getOptionValue } = require('internal/options');
const shouldAbortOnUncaughtException = getOptionValue(
  '--abort-on-uncaught-exception',
);
const { exitCodes: { kGenericUserError } } = internalBinding('errors');

const { abort, exit, _rawDebug } = process;
// #endregion

// #region constants
// From https://url.spec.whatwg.org/#special-scheme
const kSpecialSchemes = new SafeSet([
  'file:',
  'ftp:',
  'http:',
  'https:',
  'ws:',
  'wss:',
]);

/**
 * @type {symbol}
 */
const kCascade = Symbol('cascade');
/**
 * @type {symbol}
let debug = require('internal/util/debuglog').debuglog('policy', (fn) => {
  debug = fn;
});
const SRI = require('internal/policy/sri');
const { URL } = require('internal/url');
const { internalVerifyIntegrity } = internalBinding('crypto');
const kRelativeURLStringPattern = /^\.{0,2}\//;
const { getOptionValue } = require('internal/options');
const shouldAbortOnUncaughtException = getOptionValue(
  '--abort-on-uncaught-exception',
);
const { exitCodes: { kGenericUserError } } = internalBinding('errors');

const { abort, exit, _rawDebug } = process;
// #endregion

// #region constants
// From https://url.spec.whatwg.org/#special-scheme
const kSpecialSchemes = new SafeSet([
  'file:',
  'ftp:',
  'http:',
  'https:',
  'ws:',
  'wss:',
]);

/**
 * @type {symbol}
 */
const kCascade = Symbol('cascade');
/**
 * @type {symbol}
 */
const kFallThrough = Symbol('fall through');

function REACTION_THROW(error) {
  throw error;
}

function REACTION_EXIT(error) {
  REACTION_LOG(error);
  if (shouldAbortOnUncaughtException) {
    abort();
  }
  exit(kGenericUserError);
}

function REACTION_LOG(error) {
  _rawDebug(error.stack);
}

// #endregion

// #region DependencyMapperInstance
class DependencyMapperInstance {
  /**
   * @type {string}
   */
  href;
  /**
   * @type {DependencyMap | undefined}
   */
  #dependencies;
  /**
   * @type {PatternDependencyMap | undefined}
   */
  #patternDependencies;
  /**
   * @type {DependencyMapperInstance | null | undefined}
   */
  #parentDependencyMapper;
  /**
   * @type {boolean}
   */
  #normalized = false;
  /**
   * @type {boolean}
   */
  cascade;
  /**
   * @type {boolean}
   */
  allowSameHREFScope;
  /**
   * @param {string} parentHREF
   * @param {DependencyMap | undefined} dependencies
   * @param {boolean} cascade
   * @param {boolean} allowSameHREFScope
   */
  constructor(
    parentHREF,
    dependencies,
    cascade = false,
    allowSameHREFScope = false) {
    this.href = parentHREF;
    if (dependencies === kFallThrough ||
        dependencies === undefined ||
        dependencies === null) {
      this.#dependencies = dependencies;
      this.#patternDependencies = undefined;
    } else {
      const patterns = [];
      const keys = ObjectKeys(dependencies);
      for (let i = 0; i < keys.length; i++) {
        const key = keys[i];
        if (StringPrototypeEndsWith(key, '*')) {
          const target = RegExpPrototypeExec(/^([^*]*)\*([^*]*)$/);
          if (!target) {
            throw new ERR_MANIFEST_INVALID_SPECIFIER(
              this.href,
              `${target}, pattern needs to have a single trailing "*" in target`,
            );
          }
          const prefix = target[1];
          const suffix = target[2];
          patterns.push([
            target.slice(0, -1),
            [prefix, suffix],
          ]);
        }
      }
      ArrayPrototypeSort(patterns, (a, b) => {
        return a[0] < b[0] ? -1 : 1;
      });
      this.#dependencies = dependencies;
      this.#patternDependencies = patterns;
    }
    this.cascade = cascade;
    this.allowSameHREFScope = allowSameHREFScope;
    ObjectFreeze(this);
  }
  /**
   *
   * @param {string} normalizedSpecifier
   * @param {Set<string>} conditions
   * @param {Manifest} manifest
   * @returns {URL | typeof kFallThrough | null}
   */
  _resolveAlreadyNormalized(normalizedSpecifier, conditions, manifest) {
    let dependencies = this.#dependencies;
    debug(this.href, 'resolving', normalizedSpecifier);
    if (dependencies === kFallThrough) return true;
    if (dependencies !== undefined && typeof dependencies === 'object') {
      const normalized = this.#normalized;
      if (normalized !== true) {
        /**
         * @type {Record<string, string>}
         */
        const normalizedDependencyMap = { __proto__: null };
        for (let specifier in dependencies) {
          const target = dependencies[specifier];
          specifier = canonicalizeSpecifier(specifier, manifest.href);
          normalizedDependencyMap[specifier] = target;
        }
        ObjectFreeze(normalizedDependencyMap);
        dependencies = normalizedDependencyMap;
        this.#dependencies = normalizedDependencyMap;
        this.#normalized = true;
      }
      debug(dependencies);
      if (normalizedSpecifier in dependencies === true) {
        const to = searchDependencies(
          this.href,
          dependencies[normalizedSpecifier],
          conditions,
        );
        debug({ to });
        if (to === true) {
          return true;
        }
        let ret;
        if (parsedURLs && parsedURLs.has(to)) {
          ret = parsedURLs.get(to);
        } else if (RegExpPrototypeExec(kRelativeURLStringPattern, to) !== null) {
          ret = resolve(to, manifest.href);
        } else {
          ret = resolve(to);
        }
        return ret;
      }
    }
    const { cascade } = this;
    if (cascade !== true) {
      return null;
    }
    let parentDependencyMapper = this.#parentDependencyMapper;
    if (parentDependencyMapper === undefined) {
      parentDependencyMapper = manifest.getScopeDependencyMapper(
        this.href,
        this.allowSameHREFScope,
      );
      this.#parentDependencyMapper = parentDependencyMapper;
    }
    if (parentDependencyMapper === null) {
      return null;
    }
    return parentDependencyMapper._resolveAlreadyNormalized(
      normalizedSpecifier,
      conditions,
      manifest,
    );
  }
}

const kArbitraryDependencies = new DependencyMapperInstance(
  'arbitrary dependencies',
  kFallThrough,
  false,
  true,
);
const kNoDependencies = new DependencyMapperInstance(
  'no dependencies',
  null,
  false,
  true,
);
/**
 * @param {string} href
 * @param {JSONDependencyMap} dependencies
 * @param {boolean} cascade
 * @param {boolean} allowSameHREFScope
 * @param {Map<string | null | undefined, DependencyMapperInstance>} store
 */
const insertDependencyMap = (
  href,
  dependencies,
  cascade,
  allowSameHREFScope,
  store,
) => {
  if (cascade !== undefined && typeof cascade !== 'boolean') {
    throw new ERR_MANIFEST_INVALID_RESOURCE_FIELD(href, 'cascade');
  }
  if (dependencies === true) {
    store.set(href, kArbitraryDependencies);
    return;
  }
  if (dependencies === null || dependencies === undefined) {
    store.set(
      href,
      cascade ?
        new DependencyMapperInstance(href, null, true, allowSameHREFScope) :
        kNoDependencies,
    );
    return;
  }
  if (objectButNotArray(dependencies)) {
    store.set(
      href,
      new DependencyMapperInstance(
        href,
        dependencies,
        cascade,
        allowSameHREFScope,
      ),
    );
    return;
  }
  throw new ERR_MANIFEST_INVALID_RESOURCE_FIELD(href, 'dependencies');
};
/**
 * Finds the longest key within `this.#scopeDependencies` that covers a
 * specific HREF
 * @param {string} href
 * @param {ScopeStore} scopeStore
 * @returns {null | string}
 */
function findScopeHREF(href, scopeStore, allowSame) {
  let protocol;
  if (href !== '') {
    // default URL parser does some stuff to special urls... skip if this is
    // just the protocol
    if (RegExpPrototypeExec(/^[^:]*[:]$/, href) !== null) {
      protocol = href;
    } else {
      let currentURL = new URL(href);
      const normalizedHREF = currentURL.href;
      protocol = currentURL.protocol;
      // Non-opaque blobs adopt origins
      if (protocol === 'blob:' && currentURL.origin !== 'null') {
        currentURL = new URL(currentURL.origin);
        protocol = currentURL.protocol;
      }
      // Only a few schemes are hierarchical
      if (kSpecialSchemes.has(currentURL.protocol)) {
        // Make first '..' act like '.'
        if (!StringPrototypeEndsWith(currentURL.pathname, '/')) {
          currentURL.pathname += '/';
        }
        let lastHREF;
        let currentHREF = currentURL.href;
        do {
          if (scopeStore.has(currentHREF)) {
            if (allowSame || currentHREF !== normalizedHREF) {
              return currentHREF;
            }
        for (let i = 0; i < integrityEntries.length; i++) {
          const { algorithm, value: expected } = integrityEntries[i];
          // TODO(tniessen): the content should not be passed as a string in the
          // first place, see https://github.com/nodejs/node/issues/39707
          const mismatchedIntegrity = internalVerifyIntegrity(algorithm, content, expected);
          if (mismatchedIntegrity === undefined) {
            return true;
          }
          realIntegrities.set(algorithm, mismatchedIntegrity);
        }