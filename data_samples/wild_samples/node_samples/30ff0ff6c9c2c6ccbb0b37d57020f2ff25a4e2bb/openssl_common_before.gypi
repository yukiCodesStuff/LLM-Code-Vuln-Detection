      'xcode_settings': {
        'WARNING_CFLAGS': ['-Wno-missing-field-initializers']
      },
      'defines': [
        'ENGINESDIR="/dev/null"',
      ],
    }, 'OS=="solaris"', {
      'defines': [
        'OPENSSLDIR="/etc/ssl"',
        'ENGINESDIR="/dev/null"',
        '__EXTENSIONS__'
      ],
    }, {
      # linux and others
      'cflags': ['-Wno-missing-field-initializers',],
      'defines': [
        'OPENSSLDIR="/etc/ssl"',
        'ENGINESDIR="/dev/null"',
        'TERMIOS',
      ],
      'conditions': [
        [ 'llvm_version=="0.0"', {
          'cflags': ['-Wno-old-style-declaration',],
        }],
      ],
    }],
  ]
}