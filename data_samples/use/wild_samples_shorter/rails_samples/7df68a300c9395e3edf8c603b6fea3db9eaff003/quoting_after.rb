          case value
          when Range
            if /range$/ =~ sql_type
              escaped = quote_string(PostgreSQLColumn.range_to_string(value))
              "#{escaped}::#{sql_type}"
            else
              super
            end
          when Hash
            when 'xml'   then "xml '#{quote_string(value)}'"
            when /^bit/
              case value
              when /\A[01]*\Z/      then "B'#{value}'" # Bit-string notation
              when /\A[0-9A-F]*\Z/i then "X'#{value}'" # Hexadecimal notation
              end
            else
              super
            end