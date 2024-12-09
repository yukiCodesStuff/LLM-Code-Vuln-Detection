            end
          end

          def quote_and_escape(value)
            case value
            when "NULL"
              value
            else
              "\"#{value.gsub(/"/,"\\\"")}\""
            end
          end

          def type_cast_array(oid, value)