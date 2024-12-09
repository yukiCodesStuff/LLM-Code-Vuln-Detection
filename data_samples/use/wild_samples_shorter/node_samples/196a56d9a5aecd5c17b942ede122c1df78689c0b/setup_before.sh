# cross root and root cross cert
./mkcert.sh genroot "Cross Root" cross-key cross-root
./mkcert.sh genca "Root CA" root-key root-cross-cert cross-key cross-root
# trust variants: +serverAuth -serverAuth +clientAuth -clientAuth,
openssl x509 -in root-cert.pem -trustout \
    -addtrust serverAuth -out root+serverAuth.pem
openssl x509 -in root-cert.pem -trustout \
    -addreject serverAuth -out root-serverAuth.pem

# Primary intermediate ca: ca-cert
./mkcert.sh genca "CA" ca-key ca-cert root-key root-cert
# ca variants: CA:false, key2, DN2, issuer2, expired
./mkcert.sh genee "CA" ca-key ca-nonca root-key root-cert
./mkcert.sh gen_nonbc_ca "CA" ca-key ca-nonbc root-key root-cert
./mkcert.sh genca "CA" ca-key2 ca-cert2 root-key root-cert
./mkcert.sh genca "CA2" ca-key ca-name2 root-key root-cert