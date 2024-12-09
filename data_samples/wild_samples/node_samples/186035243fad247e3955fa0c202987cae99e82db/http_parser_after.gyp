      'direct_dependent_settings': {
        'defines': [ 'HTTP_PARSER_STRICT=0' ],
        'include_dirs': [ '.' ],
      },
      'defines': [ 'HTTP_MAX_HEADER_SIZE=8192', 'HTTP_PARSER_STRICT=0' ],
      'sources': [ './http_parser.c', ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              # Compile as C++. http_parser.c is actually C99, but C++ is
              # close enough in this case.
              'CompileAs': 2,
            },
          },
        }]
      ],
    },

    {
      'target_name': 'http_parser_strict',
      'type': 'static_library',
      'include_dirs': [ '.' ],
      'direct_dependent_settings': {
        'defines': [ 'HTTP_PARSER_STRICT=1' ],
        'include_dirs': [ '.' ],
      },
      'defines': [ 'HTTP_MAX_HEADER_SIZE=8192', 'HTTP_PARSER_STRICT=1' ],
      'sources': [ './http_parser.c', ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              # Compile as C++. http_parser.c is actually C99, but C++ is
              # close enough in this case.
              'CompileAs': 2,
            },
          },
        }]
      ],
    },

    {
      'target_name': 'test-nonstrict',
      'type': 'executable',
      'dependencies': [ 'http_parser' ],
      'sources': [ 'test.c' ]
    },

    {
      'target_name': 'test-strict',
      'type': 'executable',
      'dependencies': [ 'http_parser_strict' ],
      'sources': [ 'test.c' ]
    }
  ]
}
      'direct_dependent_settings': {
        'defines': [ 'HTTP_PARSER_STRICT=1' ],
        'include_dirs': [ '.' ],
      },
      'defines': [ 'HTTP_MAX_HEADER_SIZE=8192', 'HTTP_PARSER_STRICT=1' ],
      'sources': [ './http_parser.c', ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              # Compile as C++. http_parser.c is actually C99, but C++ is
              # close enough in this case.
              'CompileAs': 2,
            },
          },
        }]
      ],
    },

    {
      'target_name': 'test-nonstrict',
      'type': 'executable',
      'dependencies': [ 'http_parser' ],
      'sources': [ 'test.c' ]
    },

    {
      'target_name': 'test-strict',
      'type': 'executable',
      'dependencies': [ 'http_parser_strict' ],
      'sources': [ 'test.c' ]
    }
  ]
}