  if (self.destroyed) {
    this.buffer = null;
    this.cb();
    return;
  }

  const availOutAfter = state[0];
  const availInAfter = state[1];

  const inDelta = handle.availInBefore - availInAfter;
  self.bytesWritten += inDelta;

  const have = handle.availOutBefore - availOutAfter;
  if (have > 0) {
    const out = self._outBuffer.slice(self._outOffset, self._outOffset + have);
    self._outOffset += have;
    self.push(out);
  } else {
    assert(have === 0, 'have should not go down');
  }

  if (self.destroyed) {
    this.cb();
    return;
  }

  // Exhausted the output buffer, or used all the input create a new one.
  if (availOutAfter === 0 || self._outOffset >= self._chunkSize) {
    handle.availOutBefore = self._chunkSize;
    self._outOffset = 0;
    self._outBuffer = Buffer.allocUnsafe(self._chunkSize);
  }

  if (availOutAfter === 0) {
    // Not actually done. Need to reprocess.
    // Also, update the availInBefore to the availInAfter value,
    // so that if we have to hit it a third (fourth, etc.) time,
    // it'll have the correct byte counts.
    handle.inOff += inDelta;
    handle.availInBefore = availInAfter;

    this.write(handle.flushFlag,
               this.buffer, // in
               handle.inOff, // in_off
               handle.availInBefore, // in_len
               self._outBuffer, // out
               self._outOffset, // out_off
               self._chunkSize); // out_len
    return;
  }

  if (availInAfter > 0) {
    // If we have more input that should be written, but we also have output
    // space available, that means that the compression library was not
    // interested in receiving more data, and in particular that the input
    // stream has ended early.
    // This applies to streams where we don't check data past the end of
    // what was consumed; that is, everything except Gunzip/Unzip.
    self.push(null);
  }

  // Finished with the chunk.
  this.buffer = null;
  this.cb();
}
  if (availOutAfter === 0) {
    // Not actually done. Need to reprocess.
    // Also, update the availInBefore to the availInAfter value,
    // so that if we have to hit it a third (fourth, etc.) time,
    // it'll have the correct byte counts.
    handle.inOff += inDelta;
    handle.availInBefore = availInAfter;

    this.write(handle.flushFlag,
               this.buffer, // in
               handle.inOff, // in_off
               handle.availInBefore, // in_len
               self._outBuffer, // out
               self._outOffset, // out_off
               self._chunkSize); // out_len
    return;
  }

  if (availInAfter > 0) {
    // If we have more input that should be written, but we also have output
    // space available, that means that the compression library was not
    // interested in receiving more data, and in particular that the input
    // stream has ended early.
    // This applies to streams where we don't check data past the end of
    // what was consumed; that is, everything except Gunzip/Unzip.
    self.push(null);
  }

  // Finished with the chunk.
  this.buffer = null;
  this.cb();
}

function _close(engine) {
  // Caller may invoke .close after a zlib error (which will null _handle).
  if (!engine._handle)
    return;

  engine._handle.close();
  engine._handle = null;
}

const zlibDefaultOpts = {
  flush: Z_NO_FLUSH,
  finishFlush: Z_FINISH,
  fullFlush: Z_FULL_FLUSH,
};
// Base class for all streams actually backed by zlib and using zlib-specific
// parameters.
function Zlib(opts, mode) {
  let windowBits = Z_DEFAULT_WINDOWBITS;
  let level = Z_DEFAULT_COMPRESSION;
  let memLevel = Z_DEFAULT_MEMLEVEL;
  let strategy = Z_DEFAULT_STRATEGY;
  let dictionary;

  if (opts) {
    // windowBits is special. On the compression side, 0 is an invalid value.
    // But on the decompression side, a value of 0 for windowBits tells zlib
    // to use the window size in the zlib header of the compressed stream.
    if ((opts.windowBits == null || opts.windowBits === 0) &&
        (mode === INFLATE ||
         mode === GUNZIP ||
         mode === UNZIP)) {
      windowBits = 0;
    } else {
      // `{ windowBits: 8 }` is valid for deflate but not gzip.
      const min = Z_MIN_WINDOWBITS + (mode === GZIP ? 1 : 0);
      windowBits = checkRangesOrGetDefault(
        opts.windowBits, 'options.windowBits',
        min, Z_MAX_WINDOWBITS, Z_DEFAULT_WINDOWBITS);
    }

    level = checkRangesOrGetDefault(
      opts.level, 'options.level',
      Z_MIN_LEVEL, Z_MAX_LEVEL, Z_DEFAULT_COMPRESSION);

    memLevel = checkRangesOrGetDefault(
      opts.memLevel, 'options.memLevel',
      Z_MIN_MEMLEVEL, Z_MAX_MEMLEVEL, Z_DEFAULT_MEMLEVEL);

    strategy = checkRangesOrGetDefault(
      opts.strategy, 'options.strategy',
      Z_DEFAULT_STRATEGY, Z_FIXED, Z_DEFAULT_STRATEGY);

    dictionary = opts.dictionary;
    if (dictionary !== undefined && !isArrayBufferView(dictionary)) {
      if (isAnyArrayBuffer(dictionary)) {
        dictionary = Buffer.from(dictionary);
      } else {
        throw new ERR_INVALID_ARG_TYPE(
          'options.dictionary',
          ['Buffer', 'TypedArray', 'DataView', 'ArrayBuffer'],
          dictionary,
        );
      }
    }
  }

  const handle = new binding.Zlib(mode);
  // Ideally, we could let ZlibBase() set up _writeState. I haven't been able
  // to come up with a good solution that doesn't break our internal API,
  // and with it all supported npm versions at the time of writing.
  this._writeState = new Uint32Array(2);
  handle.init(windowBits,
              level,
              memLevel,
              strategy,
              this._writeState,
              processCallback,
              dictionary);

  ReflectApply(ZlibBase, this, [opts, mode, handle, zlibDefaultOpts]);

  this._level = level;
  this._strategy = strategy;
}
ObjectSetPrototypeOf(Zlib.prototype, ZlibBase.prototype);
ObjectSetPrototypeOf(Zlib, ZlibBase);

// This callback is used by `.params()` to wait until a full flush happened
// before adjusting the parameters. In particular, the call to the native
// `params()` function should not happen while a write is currently in progress
// on the threadpool.
function paramsAfterFlushCallback(level, strategy, callback) {
  assert(this._handle, 'zlib binding closed');
  this._handle.params(level, strategy);
  if (!this.destroyed) {
    this._level = level;
    this._strategy = strategy;
    if (callback) callback();
  }
}

Zlib.prototype.params = function params(level, strategy, callback) {
  checkRangesOrGetDefault(level, 'level', Z_MIN_LEVEL, Z_MAX_LEVEL);
  checkRangesOrGetDefault(strategy, 'strategy', Z_DEFAULT_STRATEGY, Z_FIXED);

  if (this._level !== level || this._strategy !== strategy) {
    this.flush(Z_SYNC_FLUSH,
               FunctionPrototypeBind(paramsAfterFlushCallback, this,
                                     level, strategy, callback));
  } else {
    process.nextTick(callback);
  }
};

// generic zlib
// minimal 2-byte header
function Deflate(opts) {
  if (!(this instanceof Deflate))
    return new Deflate(opts);
  ReflectApply(Zlib, this, [opts, DEFLATE]);
}
ObjectSetPrototypeOf(Deflate.prototype, Zlib.prototype);
ObjectSetPrototypeOf(Deflate, Zlib);

function Inflate(opts) {
  if (!(this instanceof Inflate))
    return new Inflate(opts);
  ReflectApply(Zlib, this, [opts, INFLATE]);
}
ObjectSetPrototypeOf(Inflate.prototype, Zlib.prototype);
ObjectSetPrototypeOf(Inflate, Zlib);

function Gzip(opts) {
  if (!(this instanceof Gzip))
    return new Gzip(opts);
  ReflectApply(Zlib, this, [opts, GZIP]);
}
ObjectSetPrototypeOf(Gzip.prototype, Zlib.prototype);
ObjectSetPrototypeOf(Gzip, Zlib);

function Gunzip(opts) {
  if (!(this instanceof Gunzip))
    return new Gunzip(opts);
  ReflectApply(Zlib, this, [opts, GUNZIP]);
}
ObjectSetPrototypeOf(Gunzip.prototype, Zlib.prototype);
ObjectSetPrototypeOf(Gunzip, Zlib);

function DeflateRaw(opts) {
  if (opts && opts.windowBits === 8) opts.windowBits = 9;
  if (!(this instanceof DeflateRaw))
    return new DeflateRaw(opts);
  ReflectApply(Zlib, this, [opts, DEFLATERAW]);
}
ObjectSetPrototypeOf(DeflateRaw.prototype, Zlib.prototype);
ObjectSetPrototypeOf(DeflateRaw, Zlib);

function InflateRaw(opts) {
  if (!(this instanceof InflateRaw))
    return new InflateRaw(opts);
  ReflectApply(Zlib, this, [opts, INFLATERAW]);
}
ObjectSetPrototypeOf(InflateRaw.prototype, Zlib.prototype);
ObjectSetPrototypeOf(InflateRaw, Zlib);

function Unzip(opts) {
  if (!(this instanceof Unzip))
    return new Unzip(opts);
  ReflectApply(Zlib, this, [opts, UNZIP]);
}
ObjectSetPrototypeOf(Unzip.prototype, Zlib.prototype);
ObjectSetPrototypeOf(Unzip, Zlib);

function createConvenienceMethod(ctor, sync) {
  if (sync) {
    return function syncBufferWrapper(buffer, opts) {
      return zlibBufferSync(new ctor(opts), buffer);
    };
  }
  return function asyncBufferWrapper(buffer, opts, callback) {
    if (typeof opts === 'function') {
      callback = opts;
      opts = {};
    }
    return zlibBuffer(new ctor(opts), buffer, callback);
  };
}

const kMaxBrotliParam = MathMaxApply(ArrayPrototypeMap(
  ObjectKeys(constants),
  (key) => (StringPrototypeStartsWith(key, 'BROTLI_PARAM_') ?
    constants[key] :
    0),
));

const brotliInitParamsArray = new Uint32Array(kMaxBrotliParam + 1);

const brotliDefaultOpts = {
  flush: BROTLI_OPERATION_PROCESS,
  finishFlush: BROTLI_OPERATION_FINISH,
  fullFlush: BROTLI_OPERATION_FLUSH,
};
function Brotli(opts, mode) {
  assert(mode === BROTLI_DECODE || mode === BROTLI_ENCODE);

  TypedArrayPrototypeFill(brotliInitParamsArray, -1);
  if (opts?.params) {
    ArrayPrototypeForEach(ObjectKeys(opts.params), (origKey) => {
      const key = +origKey;
      if (NumberIsNaN(key) || key < 0 || key > kMaxBrotliParam ||
          (brotliInitParamsArray[key] | 0) !== -1) {
        throw new ERR_BROTLI_INVALID_PARAM(origKey);
      }

      const value = opts.params[origKey];
      if (typeof value !== 'number' && typeof value !== 'boolean') {
        throw new ERR_INVALID_ARG_TYPE('options.params[key]',
                                       'number', opts.params[origKey]);
      }
      brotliInitParamsArray[key] = value;
    });
  }

  const handle = mode === BROTLI_DECODE ?
    new binding.BrotliDecoder(mode) : new binding.BrotliEncoder(mode);

  this._writeState = new Uint32Array(2);
  // TODO(addaleax): Sometimes we generate better error codes in C++ land,
  // e.g. ERR_BROTLI_PARAM_SET_FAILED -- it's hard to access them with
  // the current bindings setup, though.
  if (!handle.init(brotliInitParamsArray,
                   this._writeState,
                   processCallback)) {
    throw new ERR_ZLIB_INITIALIZATION_FAILED();
  }

  ReflectApply(ZlibBase, this, [opts, mode, handle, brotliDefaultOpts]);
}
ObjectSetPrototypeOf(Brotli.prototype, Zlib.prototype);
ObjectSetPrototypeOf(Brotli, Zlib);

function BrotliCompress(opts) {
  if (!(this instanceof BrotliCompress))
    return new BrotliCompress(opts);
  ReflectApply(Brotli, this, [opts, BROTLI_ENCODE]);
}
ObjectSetPrototypeOf(BrotliCompress.prototype, Brotli.prototype);
ObjectSetPrototypeOf(BrotliCompress, Brotli);

function BrotliDecompress(opts) {
  if (!(this instanceof BrotliDecompress))
    return new BrotliDecompress(opts);
  ReflectApply(Brotli, this, [opts, BROTLI_DECODE]);
}
ObjectSetPrototypeOf(BrotliDecompress.prototype, Brotli.prototype);
ObjectSetPrototypeOf(BrotliDecompress, Brotli);


function createProperty(ctor) {
  return {
    __proto__: null,
    configurable: true,
    enumerable: true,
    value: function(options) {
      return new ctor(options);
    },
  };
}

// Legacy alias on the C++ wrapper object. This is not public API, so we may
// want to runtime-deprecate it at some point. There's no hurry, though.
ObjectDefineProperty(binding.Zlib.prototype, 'jsref', {
  __proto__: null,
  get() { return this[owner_symbol]; },
  set(v) { return this[owner_symbol] = v; },
});

module.exports = {
  Deflate,
  Inflate,
  Gzip,
  Gunzip,
  DeflateRaw,
  InflateRaw,
  Unzip,
  BrotliCompress,
  BrotliDecompress,

  // Convenience methods.
  // compress/decompress a string or buffer in one step.
  deflate: createConvenienceMethod(Deflate, false),
  deflateSync: createConvenienceMethod(Deflate, true),
  gzip: createConvenienceMethod(Gzip, false),
  gzipSync: createConvenienceMethod(Gzip, true),
  deflateRaw: createConvenienceMethod(DeflateRaw, false),
  deflateRawSync: createConvenienceMethod(DeflateRaw, true),
  unzip: createConvenienceMethod(Unzip, false),
  unzipSync: createConvenienceMethod(Unzip, true),
  inflate: createConvenienceMethod(Inflate, false),
  inflateSync: createConvenienceMethod(Inflate, true),
  gunzip: createConvenienceMethod(Gunzip, false),
  gunzipSync: createConvenienceMethod(Gunzip, true),
  inflateRaw: createConvenienceMethod(InflateRaw, false),
  inflateRawSync: createConvenienceMethod(InflateRaw, true),
  brotliCompress: createConvenienceMethod(BrotliCompress, false),
  brotliCompressSync: createConvenienceMethod(BrotliCompress, true),
  brotliDecompress: createConvenienceMethod(BrotliDecompress, false),
  brotliDecompressSync: createConvenienceMethod(BrotliDecompress, true),
};

ObjectDefineProperties(module.exports, {
  createDeflate: createProperty(Deflate),
  createInflate: createProperty(Inflate),
  createDeflateRaw: createProperty(DeflateRaw),
  createInflateRaw: createProperty(InflateRaw),
  createGzip: createProperty(Gzip),
  createGunzip: createProperty(Gunzip),
  createUnzip: createProperty(Unzip),
  createBrotliCompress: createProperty(BrotliCompress),
  createBrotliDecompress: createProperty(BrotliDecompress),
  constants: {
    __proto__: null,
    configurable: false,
    enumerable: true,
    value: constants,
  },
  codes: {
    __proto__: null,
    enumerable: true,
    writable: false,
    value: ObjectFreeze(codes),
  },
});

// These should be considered deprecated
// expose all the zlib constants
for (const bkey of ObjectKeys(constants)) {
  if (StringPrototypeStartsWith(bkey, 'BROTLI')) continue;
  ObjectDefineProperty(module.exports, bkey, {
    __proto__: null,
    enumerable: false, value: constants[bkey], writable: false,
  });
}