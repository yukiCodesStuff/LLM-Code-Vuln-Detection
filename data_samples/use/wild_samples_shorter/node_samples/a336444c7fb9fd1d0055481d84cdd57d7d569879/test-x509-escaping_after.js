  }
}

// Test escaping rules for the subject field.
{
  const expectedSubjects = [
    {
      text: 'L=Somewhere\nCN=evil.example.com',
      legacy: {
        L: 'Somewhere',
        CN: 'evil.example.com',
      },
    },
    {
      text: 'L=Somewhere\\00evil.example.com',
      legacy: {
        L: 'Somewhere\0evil.example.com',
      },
    },
    {
      text: 'L=Somewhere\\0ACN=evil.example.com',
      legacy: {
        L: 'Somewhere\nCN=evil.example.com'
      },
    },
    {
      text: 'L=Somewhere\\, CN = evil.example.com',
      legacy: {
        L: 'Somewhere, CN = evil.example.com'
      },
    },
    {
      text: 'L=Somewhere/CN=evil.example.com',
      legacy: {
        L: 'Somewhere/CN=evil.example.com'
      },
    },
    {
      text: 'L=München\\\\\\0ACN=evil.example.com',
      legacy: {
        L: 'München\\\nCN=evil.example.com'
      }
    },
    {
      text: 'L=Somewhere + CN=evil.example.com',
      legacy: {
        L: 'Somewhere',
        CN: 'evil.example.com',
      }
    },
    {
      text: 'L=Somewhere \\+ CN=evil.example.com',
      legacy: {
        L: 'Somewhere + CN=evil.example.com'
      }
    },
    // Observe that the legacy representation cannot properly distinguish
    // between multi-value RDNs and multiple single-value RDNs.
    {
      text: 'L=L1 + L=L2\nL=L3',
      legacy: {
        L: ['L1', 'L2', 'L3']
      },
    },
    {
      text: 'L=L1\nL=L2\nL=L3',
      legacy: {
        L: ['L1', 'L2', 'L3']
      },
    },
  ];

  const serverKey = fixtures.readSync('x509-escaping/server-key.pem', 'utf8');

  for (let i = 0; i < expectedSubjects.length; i++) {
    const pem = fixtures.readSync(`x509-escaping/subj-${i}-cert.pem`, 'utf8');
    const expected = expectedSubjects[i];

    // Test the subject property of the X509Certificate API.
    const cert = new X509Certificate(pem);
    assert.strictEqual(cert.subject, expected.text);
    // The issuer MUST be the same as the subject since the cert is self-signed.
    assert.strictEqual(cert.issuer, expected.text);

    // Test that the certificate obtained by checkServerIdentity has the correct
    // subject property.
    const server = tls.createServer({
      key: serverKey,
      cert: pem,
    }, common.mustCall((conn) => {
      conn.destroy();
      server.close();
    })).listen(common.mustCall(() => {
      const { port } = server.address();
      tls.connect(port, {
        ca: pem,
        servername: 'example.com',
        checkServerIdentity: (hostname, peerCert) => {
          assert.strictEqual(hostname, 'example.com');
          const expectedObject = Object.assign(Object.create(null),
                                               expected.legacy);
          assert.deepStrictEqual(peerCert.subject, expectedObject);
          // The issuer MUST be the same as the subject since the cert is
          // self-signed. Otherwise, OpenSSL would have already rejected the
          // certificate while connecting to the TLS server.
          assert.deepStrictEqual(peerCert.issuer, expectedObject);
        },
      }, common.mustCall());
    }));
  }
}

// The internal parsing logic must match the JSON specification exactly.
{
  // This list is partially based on V8's own JSON tests.
  const invalidJSON = [