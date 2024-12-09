  const numLeaves = 5;

  for (let i = 0; i < numLeaves; i++) {
    const name = `x509-escaping/google/leaf${i}.pem`;
    const leafPEM = fixtures.readSync(name, 'utf8');

    const server = tls.createServer({
    }));
  })).unref();
}

// The subject MUST NOT be ignored if no dNSName subject alternative name
// exists, even if other subject alternative names exist.
{
  const key = fixtures.readKey('irrelevant_san_correct_subject-key.pem');
  const cert = fixtures.readKey('irrelevant_san_correct_subject-cert.pem');

  // The hostname is the CN, but there is no dNSName SAN entry.
  const servername = 'good.example.com';
  const certX509 = new X509Certificate(cert);
  assert.strictEqual(certX509.subject, `CN=${servername}`);
  assert.strictEqual(certX509.subjectAltName, 'IP Address:1.2.3.4');

  // Connect to a server that uses the self-signed certificate.
  const server = tls.createServer({ key, cert }, common.mustCall((socket) => {
    socket.destroy();
    server.close();
  })).listen(common.mustCall(() => {
    const { port } = server.address();
    tls.connect(port, {
      ca: cert,
      servername,
    }, common.mustCall(() => {
      // Do nothing, the server will close the connection.
    }));
  }));
}