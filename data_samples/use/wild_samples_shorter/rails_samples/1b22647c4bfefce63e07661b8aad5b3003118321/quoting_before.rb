  module ConnectionAdapters
    module PostgreSQL
      module Quoting
        # Escapes binary strings for bytea input to the database.
        def escape_bytea(value)
          valid_raw_connection.escape_bytea(value) if value
        end
          valid_raw_connection.unescape_bytea(value) if value
        end

        def quote(value) # :nodoc:
          case value
          when OID::Xml::Data
            "xml '#{quote_string(value.to_s)}'"
          when OID::Bit::Data