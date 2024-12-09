          case value
          when Range
            if /range$/ =~ sql_type
              "'#{PostgreSQLColumn.range_to_string(value)}'::#{sql_type}"
            else
              super
            end
          when Hash
            when 'xml'   then "xml '#{quote_string(value)}'"
            when /^bit/
              case value
              when /^[01]*$/      then "B'#{value}'" # Bit-string notation
              when /^[0-9A-F]*$/i then "X'#{value}'" # Hexadecimal notation
              end
            else
              super
            end