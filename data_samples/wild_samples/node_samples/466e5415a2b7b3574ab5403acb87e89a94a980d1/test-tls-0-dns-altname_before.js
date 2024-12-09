  }, common.mustCall(() => {
    const cert = c.getPeerCertificate();
    assert.strictEqual(cert.subjectaltname,
                       'DNS:good.example.org\0.evil.example.com, ' +
                           'DNS:just-another.example.com, ' +
                           'IP Address:8.8.8.8, ' +
                           'IP Address:8.8.4.4, ' +
                           'DNS:last.example.com');
    c.write('ok');
  }));
}));