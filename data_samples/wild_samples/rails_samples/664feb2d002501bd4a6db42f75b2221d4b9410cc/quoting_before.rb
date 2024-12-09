module ActiveRecord
  module ConnectionAdapters
    module PostgreSQL
      module Quoting
        # Escapes binary strings for bytea input to the database.
        def escape_bytea(value)
          @connection.escape_bytea(value) if value
        end

        # Unescapes bytea output from a database to the binary string it represents.
        # NOTE: This is NOT an inverse of escape_bytea! This is only to be used
        # on escaped binary output from database drive.
        def unescape_bytea(value)
          @connection.unescape_bytea(value) if value
        end

        # Quotes PostgreSQL-specific data types for SQL input.
        def quote(value, column = nil) #:nodoc:
          return super unless column

          sql_type = type_to_sql(column.type, column.limit, column.precision, column.scale)

          case value
          when Range
            if /range$/ =~ sql_type
              escaped = quote_string(PostgreSQLColumn.range_to_string(value))
              "#{escaped}::#{sql_type}"
            else
              super
            end
          when Hash
            case sql_type
            when 'hstore' then super(PostgreSQLColumn.hstore_to_string(value), column)
            else super
            end
          when Float
            if value.infinite? && column.type == :datetime
              "'#{value.to_s.downcase}'"
            elsif value.infinite? || value.nan?
              "'#{value.to_s}'"
            else
              super
            end
          when Numeric
            if sql_type == 'money' || [:string, :text].include?(column.type)
              # Not truly string input, so doesn't require (or allow) escape string syntax.
              "'#{value}'"
            else
              super
            end
          when String
            case sql_type
            when 'xml'   then "xml '#{quote_string(value)}'"
            when /^bit/
              case value
              when /\A[01]*\Z/      then "B'#{value}'" # Bit-string notation
              when /\A[0-9A-F]*\Z/i then "X'#{value}'" # Hexadecimal notation
              end
            else
              super
            end
          else
            super
          end
        end

        def type_cast(value, column)
          return super unless column

          case value
          when Range
            if /range$/ =~ column.sql_type
              PostgreSQLColumn.range_to_string(value)
            else
              super
            end
          when NilClass
            if column.array
              value
            else
              super
            end
          when Hash
            case column.sql_type
            when 'hstore' then PostgreSQLColumn.hstore_to_string(value)
            else super
            end
          else
            super
          end
        end

        # Quotes strings for use in SQL input.
        def quote_string(s) #:nodoc:
          @connection.escape(s)
        end

        # Checks the following cases:
        #
        # - table_name
        # - "table.name"
        # - schema_name.table_name
        # - schema_name."table.name"
        # - "schema.name".table_name
        # - "schema.name"."table.name"
        def quote_table_name(name)
          Utils.extract_schema_qualified_name(name.to_s).quoted
        end

        def quote_table_name_for_assignment(table, attr)
          quote_column_name(attr)
        end

        # Quotes column names for use in SQL queries.
        def quote_column_name(name) #:nodoc:
          PGconn.quote_ident(name.to_s)
        end

        # Quote date/time values for use in SQL input. Includes microseconds
        # if the value is a Time responding to usec.
        def quoted_date(value) #:nodoc:
          result = super
          if value.acts_like?(:time) && value.respond_to?(:usec)
            result = "#{result}.#{sprintf("%06d", value.usec)}"
          end

          if value.year <= 0
            bce_year = format("%04d", -value.year + 1)
            result = result.sub(/^-?\d+/, bce_year) + " BC"
          end
          result
        end

        # Does not quote function default values for UUID columns
        def quote_default_value(value, column) #:nodoc:
          if column.type == :uuid && value =~ /\(\)/
            value
          else
            quote(value, column)
          end
        end

        private

        def _quote(value)
          if value.is_a?(Type::Binary::Data)
            "'#{escape_bytea(value.to_s)}'"
          else
            super
          end
        end

        def _type_cast(value)
          if value.is_a?(Type::Binary::Data)
            # Return a bind param hash with format as binary.
            # See http://deveiate.org/code/pg/PGconn.html#method-i-exec_prepared-doc
            # for more information
            { value: value.to_s, format: 1 }
          else
            super
          end
        end
      end
    end
  end
end