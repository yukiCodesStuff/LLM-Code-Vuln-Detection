    'conditions': [
      ['OS == "win"', {
        'obj_dir_abs': '<(PRODUCT_DIR_ABS)/obj',
        'openssl_dir': '<(PRODUCT_DIR_ABS)/obj/lib',
      }],
      ['GENERATOR == "ninja"', {
        'obj_dir_abs': '<(PRODUCT_DIR_ABS)/obj',
        'modules_dir': '<(PRODUCT_DIR_ABS)/obj/lib/openssl-modules',
        'openssl_dir': '<(PRODUCT_DIR_ABS)/obj/lib',
      }, {
        'obj_dir_abs%': '<(PRODUCT_DIR_ABS)/obj.target',
        'modules_dir': '<(PRODUCT_DIR_ABS)/obj.target/deps/openssl/lib/openssl-modules',
        'openssl_dir': '<(PRODUCT_DIR_ABS)/obj.target/deps/openssl',
      }],
      ['OS=="mac"', {
        'obj_dir_abs%': '<(PRODUCT_DIR_ABS)/obj.target',
        'modules_dir': '<(PRODUCT_DIR_ABS)/obj.target/deps/openssl/lib/openssl-modules',
        'openssl_dir': '<(PRODUCT_DIR_ABS)/obj.target/deps/openssl',
      }],
    ],
  },
  'targets': [
        ['node_shared_openssl=="false"', {
          'defines': [
            'MODULESDIR="<(modules_dir)"',
            'OPENSSLDIR="<(openssl_dir)"',
          ]
        }],
      ],
      'direct_dependent_settings': {