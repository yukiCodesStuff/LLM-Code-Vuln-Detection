  module ConnectionAdapters
    module PostgreSQL
      module Quoting
        class IntegerOutOf64BitRange < StandardError
          def initialize(msg)
            super(msg)
          end
        end

        # Escapes binary strings for bytea input to the database.
        def escape_bytea(value)
          valid_raw_connection.escape_bytea(value) if value
        end
          valid_raw_connection.unescape_bytea(value) if value
        end

        def check_int_in_range(value)
          if value.to_int > 9223372036854775807 || value.to_int < -9223372036854775808
            exception = <<~ERROR
              Provided value outside of the range of a signed 64bit integer.

              PostgreSQL will treat the column type in question as a numeric.
              This may result in a slow sequential scan due to a comparison
              being performed between an integer or bigint value and a numeric value.

              To allow for this potentially unwanted behavior, set
              ActiveRecord.raise_int_wider_than_64bit to false.
            ERROR
            raise IntegerOutOf64BitRange.new exception
          end
        end

        def quote(value) # :nodoc:
          if ActiveRecord.raise_int_wider_than_64bit && value.is_a?(Integer)
            check_int_in_range(value)
          end

          case value
          when OID::Xml::Data
            "xml '#{quote_string(value.to_s)}'"
          when OID::Bit::Data