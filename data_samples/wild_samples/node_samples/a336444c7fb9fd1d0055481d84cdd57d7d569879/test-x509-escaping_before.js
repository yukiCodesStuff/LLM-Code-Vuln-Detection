        checkServerIdentity: (hostname, peerCert) => {
          assert.strictEqual(hostname, 'example.com');
          assert.deepStrictEqual(peerCert.infoAccess,
                                 Object.assign(Object.create(null),
                                               expected.legacy));
        },
      }, common.mustCall());
    }));
  }
}

// The internal parsing logic must match the JSON specification exactly.
{
  // This list is partially based on V8's own JSON tests.
  const invalidJSON = [
    '"\\a invalid escape"',
    '"\\v invalid escape"',
    '"\\\' invalid escape"',
    '"\\x42 invalid escape"',
    '"\\u202 invalid escape"',
    '"\\012 invalid escape"',
    '"Unterminated string',
    '"Unterminated string\\"',
    '"Unterminated string\\\\\\"',
    '"\u0000 control character"',
    '"\u001e control character"',
    '"\u001f control character"',
  ];

  for (const invalidStringLiteral of invalidJSON) {
    // Usually, checkServerIdentity returns an error upon verification failure.
    // In this case, however, it should throw an error since this is not a
    // verification error. Node.js itself will never produce invalid JSON string
    // literals, so this can only happen when users construct invalid subject
    // alternative name strings (that do not follow escaping rules).
    assert.throws(() => {
      tls.checkServerIdentity('example.com', {
        subjectaltname: `DNS:${invalidStringLiteral}`,
      });
    }, {
      code: 'ERR_TLS_CERT_ALTNAME_FORMAT',
      message: 'Invalid subject alternative name string'
    });
  }
}

// While node does not produce commas within SAN entries, it should parse them
// correctly (i.e., not simply split at commas).
{
  // Regardless of the quotes, splitting this SAN string at commas would
  // cause checkServerIdentity to see 'DNS:b.example.com' and thus to accept
  // the certificate for b.example.com.
  const san = 'DNS:"a.example.com, DNS:b.example.com, DNS:c.example.com"';

  // This is what node used to do, and which is not correct!
  const hostname = 'b.example.com';
  assert.strictEqual(san.split(', ')[1], `DNS:${hostname}`);

  // The new implementation should parse the string correctly.
  const err = tls.checkServerIdentity(hostname, { subjectaltname: san });
  assert(err);
  assert.strictEqual(err.code, 'ERR_TLS_CERT_ALTNAME_INVALID');
  assert.strictEqual(err.message, 'Hostname/IP does not match certificate\'s ' +
                                  'altnames: Host: b.example.com. is not in ' +
                                  'the cert\'s altnames: DNS:"a.example.com, ' +
                                  'DNS:b.example.com, DNS:c.example.com"');
}

// The subject MUST be ignored if a dNSName subject alternative name exists.
{
  const key = fixtures.readKey('incorrect_san_correct_subject-key.pem');
  const cert = fixtures.readKey('incorrect_san_correct_subject-cert.pem');

  // The hostname is the CN, but not a SAN entry.
  const servername = 'good.example.com';
  const certX509 = new X509Certificate(cert);
  assert.strictEqual(certX509.subject, `CN=${servername}`);
  assert.strictEqual(certX509.subjectAltName, 'DNS:evil.example.com');

  // Try connecting to a server that uses the self-signed certificate.
  const server = tls.createServer({ key, cert }, common.mustNotCall());
  server.listen(common.mustCall(() => {
    const { port } = server.address();
    const socket = tls.connect(port, {
      ca: cert,
      servername,
    }, common.mustNotCall());
    socket.on('error', common.mustCall((err) => {
      assert.strictEqual(err.code, 'ERR_TLS_CERT_ALTNAME_INVALID');
      assert.strictEqual(err.message, 'Hostname/IP does not match ' +
                                      "certificate's altnames: Host: " +
                                      "good.example.com. is not in the cert's" +
                                      ' altnames: DNS:evil.example.com');
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