        'WARNING_CFLAGS': ['-Wno-missing-field-initializers']
      },
      'defines': [
        'OPENSSLDIR="/System/Library/OpenSSL/"',
        'ENGINESDIR="/dev/null"',
      ],
    }, 'OS=="solaris"', {
      'defines': [