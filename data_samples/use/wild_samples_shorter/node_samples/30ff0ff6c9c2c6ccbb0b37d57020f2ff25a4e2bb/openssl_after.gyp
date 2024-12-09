    'conditions': [
      ['OS == "win"', {
        'obj_dir_abs': '<(PRODUCT_DIR_ABS)/obj',
      }],
      ['GENERATOR == "ninja"', {
        'obj_dir_abs': '<(PRODUCT_DIR_ABS)/obj',
        'modules_dir': '<(PRODUCT_DIR_ABS)/obj/lib/openssl-modules',
      }, {
        'obj_dir_abs%': '<(PRODUCT_DIR_ABS)/obj.target',
        'modules_dir': '<(PRODUCT_DIR_ABS)/obj.target/deps/openssl/lib/openssl-modules',
      }],
      ['OS=="mac"', {
        'obj_dir_abs%': '<(PRODUCT_DIR_ABS)/obj.target',
        'modules_dir': '<(PRODUCT_DIR_ABS)/obj.target/deps/openssl/lib/openssl-modules',
      }],
    ],
  },
  'targets': [
        ['node_shared_openssl=="false"', {
          'defines': [
            'MODULESDIR="<(modules_dir)"',
          ]
        }],
      ],
      'direct_dependent_settings': {