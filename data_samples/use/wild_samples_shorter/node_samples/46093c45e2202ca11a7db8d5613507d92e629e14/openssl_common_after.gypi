      # linux and others
      'cflags': ['-Wno-missing-field-initializers',],
      'defines': [
        'OPENSSLDIR="/etc/ssl"',
        'ENGINESDIR="/dev/null"',
        'TERMIOS',
      ],
      'conditions': [