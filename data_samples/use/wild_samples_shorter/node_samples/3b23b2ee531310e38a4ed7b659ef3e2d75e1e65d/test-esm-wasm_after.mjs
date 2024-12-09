    strictEqual(code, 0);
  });

  it('should not allow code injection through export names', async () => {
    const { code, stderr, stdout } = await spawnPromisified(execPath, [
      '--no-warnings',
      '--experimental-wasm-modules',
      '--input-type=module',
      '--eval',
      `import * as wasmExports from ${JSON.stringify(fixtures.fileURL('es-modules/export-name-code-injection.wasm'))};`,
    ]);

    strictEqual(stderr, '');
    strictEqual(stdout, '');
    strictEqual(code, 0);
  });

  it('should allow non-identifier export names', async () => {
    const { code, stderr, stdout } = await spawnPromisified(execPath, [
      '--no-warnings',
      '--experimental-wasm-modules',
      '--input-type=module',
      '--eval',
      [
        'import { strictEqual } from "node:assert";',
        `import * as wasmExports from ${JSON.stringify(fixtures.fileURL('es-modules/export-name-syntax-error.wasm'))};`,
        'assert.strictEqual(wasmExports["?f!o:o<b>a[r]"]?.value, 12682);',
      ].join('\n'),
    ]);

    strictEqual(stderr, '');
    strictEqual(stdout, '');
    strictEqual(code, 0);
  });

  it('should properly escape import names as well', async () => {
    const { code, stderr, stdout } = await spawnPromisified(execPath, [
      '--no-warnings',
      '--experimental-wasm-modules',
      '--input-type=module',
      '--eval',
      [
        'import { strictEqual } from "node:assert";',
        `import * as wasmExports from ${JSON.stringify(fixtures.fileURL('es-modules/import-name.wasm'))};`,
        'assert.strictEqual(wasmExports.xor(), 12345);',
      ].join('\n'),
    ]);

    strictEqual(stderr, '');
    strictEqual(stdout, '');
    strictEqual(code, 0);
  });

  it('should emit experimental warning', async () => {
    const { code, signal, stderr } = await spawnPromisified(execPath, [
      '--experimental-wasm-modules',
      fixtures.path('es-modules/wasm-modules.mjs'),