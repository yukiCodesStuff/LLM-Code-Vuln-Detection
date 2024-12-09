  const pem = rfc5280.Certificate.encode(cert, 'pem', {
    label: 'CERTIFICATE'
  });
  writeFileSync(`./info-${i}-cert.pem`, `${pem}\n`);
}