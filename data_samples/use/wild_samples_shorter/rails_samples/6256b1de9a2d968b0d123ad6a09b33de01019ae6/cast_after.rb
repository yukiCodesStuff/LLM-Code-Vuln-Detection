            end
          end

          ARRAY_ESCAPE = "\\" * 2 * 2 # escape the backslash twice for PG arrays

          def quote_and_escape(value)
            case value
            when "NULL"
              value
            else
              value = value.gsub(/\\/, ARRAY_ESCAPE)
              value.gsub!(/"/,"\\\"")
              "\"#{value}\""
            end
          end

          def type_cast_array(oid, value)