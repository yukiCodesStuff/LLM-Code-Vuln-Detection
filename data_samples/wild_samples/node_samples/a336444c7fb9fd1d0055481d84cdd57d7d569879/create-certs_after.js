  const pem = rfc5280.Certificate.encode(cert, 'pem', {
    label: 'CERTIFICATE'
  });
  writeFileSync(`./info-${i}-cert.pem`, `${pem}\n`);
}

const subjects = [
  [
    [
      { type: oid.localityName, value: UTF8String.encode('Somewhere') }
    ],
    [
      { type: oid.commonName, value: UTF8String.encode('evil.example.com') }
    ]
  ],
  [
    [
      {
        type: oid.localityName,
        value: UTF8String.encode('Somewhere\0evil.example.com'),
      }
    ]
  ],
  [
    [
      {
        type: oid.localityName,
        value: UTF8String.encode('Somewhere\nCN=evil.example.com')
      }
    ]
  ],
  [
    [
      {
        type: oid.localityName,
        value: UTF8String.encode('Somewhere, CN = evil.example.com')
      }
    ]
  ],
  [
    [
      {
        type: oid.localityName,
        value: UTF8String.encode('Somewhere/CN=evil.example.com')
      }
    ]
  ],
  [
    [
      {
        type: oid.localityName,
        value: UTF8String.encode('M\u00fcnchen\\\nCN=evil.example.com')
      }
    ]
  ],
  [
    [
      { type: oid.localityName, value: UTF8String.encode('Somewhere') },
      { type: oid.commonName, value: UTF8String.encode('evil.example.com') },
    ]
  ],
  [
    [
      {
        type: oid.localityName,
        value: UTF8String.encode('Somewhere + CN=evil.example.com'),
      }
    ]
  ],
  [
    [
      { type: oid.localityName, value: UTF8String.encode('L1') },
      { type: oid.localityName, value: UTF8String.encode('L2') },
    ],
    [
      { type: oid.localityName, value: UTF8String.encode('L3') },
    ]
  ],
  [
    [
      { type: oid.localityName, value: UTF8String.encode('L1') },
    ],
    [
      { type: oid.localityName, value: UTF8String.encode('L2') },
    ],
    [
      { type: oid.localityName, value: UTF8String.encode('L3') },
    ],
  ],
];

for (let i = 0; i < subjects.length; i++) {
  const tbs = {
    version: 'v3',
    serialNumber: new BN('01', 16),
    signature: {
      algorithm: oid.sha256WithRSAEncryption,
      parameters: null_
    },
    issuer: {
      type: 'rdnSequence',
      value: subjects[i]
    },
    validity: {
      notBefore: { type: 'utcTime', value: now },
      notAfter: { type: 'utcTime', value: now + days * 86400000 }
    },
    subject: {
      type: 'rdnSequence',
      value: subjects[i]
    },
    subjectPublicKeyInfo: {
      algorithm: {
        algorithm: oid.rsaEncryption,
        parameters: null_
      },
      subjectPublicKey: {
        unused: 0,
        data: publicKey
      }
    }
  };

  // Self-sign the certificate.
  const tbsDer = rfc5280.TBSCertificate.encode(tbs, 'der');
  const signature = crypto.createSign(digest).update(tbsDer).sign(privateKey);

  // Construct the signed certificate.
  const cert = {
    tbsCertificate: tbs,
    signatureAlgorithm: {
      algorithm: oid.sha256WithRSAEncryption,
      parameters: null_
    },
    signature: {
      unused: 0,
      data: signature
    }
  };

  // Store the signed certificate.
  const pem = rfc5280.Certificate.encode(cert, 'pem', {
    label: 'CERTIFICATE'
  });
  writeFileSync(`./subj-${i}-cert.pem`, `${pem}\n`);
}