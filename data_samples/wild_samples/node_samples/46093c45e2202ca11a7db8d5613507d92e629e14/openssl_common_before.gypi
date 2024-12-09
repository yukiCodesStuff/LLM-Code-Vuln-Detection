    }, {
      # linux and others
      'cflags': ['-Wno-missing-field-initializers',],
      'defines': [
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