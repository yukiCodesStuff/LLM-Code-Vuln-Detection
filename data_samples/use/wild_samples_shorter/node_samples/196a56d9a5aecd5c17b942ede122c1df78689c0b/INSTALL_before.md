
### no-{protocol}-method

    no-{ssl|ssl3|tls|tls1|tls1_1|tls1_2|tls1_3|dtls|dtls1|dtls1_2}-method

Analogous to `no-{protocol}` but in addition do not build the methods for
applications to explicitly select individual protocol versions.  Note that there
is no `no-tls1_3-method` option because there is no application method for