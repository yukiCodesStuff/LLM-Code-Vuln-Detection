  const numLeaves = 5;

  for (let i = 0; i < numLeaves; i++) {
    // TODO(tniessen): this test case requires proper handling of URI SANs,
    // which node currently does not implement.
    if (i === 3) continue;

    const name = `x509-escaping/google/leaf${i}.pem`;
    const leafPEM = fixtures.readSync(name, 'utf8');

    const server = tls.createServer({
    }));
  })).unref();
}