          when Range
            if /range$/ =~ sql_type
              escaped = quote_string(PostgreSQLColumn.range_to_string(value))
              "'#{escaped}'::#{sql_type}"
            else
              super
            end
          when Hash