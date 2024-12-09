        'defines': [ 'HTTP_PARSER_STRICT=0' ],
        'include_dirs': [ '.' ],
      },
      'defines': [ 'HTTP_MAX_HEADER_SIZE=8192', 'HTTP_PARSER_STRICT=0' ],
      'sources': [ './http_parser.c', ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
        'defines': [ 'HTTP_PARSER_STRICT=1' ],
        'include_dirs': [ '.' ],
      },
      'defines': [ 'HTTP_MAX_HEADER_SIZE=8192', 'HTTP_PARSER_STRICT=1' ],
      'sources': [ './http_parser.c', ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {