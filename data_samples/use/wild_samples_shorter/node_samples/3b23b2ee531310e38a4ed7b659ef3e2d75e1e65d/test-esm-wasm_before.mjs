    strictEqual(code, 0);
  });

  it('should emit experimental warning', async () => {
    const { code, signal, stderr } = await spawnPromisified(execPath, [
      '--experimental-wasm-modules',
      fixtures.path('es-modules/wasm-modules.mjs'),