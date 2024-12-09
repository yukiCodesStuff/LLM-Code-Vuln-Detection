          result = without_prepared_statement?(binds) ? exec_no_cache(sql, name, binds) :
                                                        exec_cache(sql, name, binds)
          ret = yield result
          result.clear
          ret
        end

        def exec_no_cache(sql, name, binds)
          log(sql, name, binds) { @connection.async_exec(sql) }
        end

        def exec_cache(sql, name, binds)
          stmt_key = prepare_statement(sql)
          type_casted_binds = binds.map { |col, val|
            [col, type_cast(val, col)]
          }

          log(sql, name, type_casted_binds, stmt_key) do
            @connection.send_query_prepared(stmt_key, type_casted_binds.map { |_, val| val })
            @connection.block
            @connection.get_last_result
          end
        rescue ActiveRecord::StatementInvalid => e
          pgerror = e.original_exception

          # Get the PG code for the failure.  Annoyingly, the code for
          # prepared statements whose return value may have changed is
          # FEATURE_NOT_SUPPORTED.  Check here for more details:
          # http://git.postgresql.org/gitweb/?p=postgresql.git;a=blob;f=src/backend/utils/cache/plancache.c#l573
          begin
            code = pgerror.result.result_error_field(PGresult::PG_DIAG_SQLSTATE)
          rescue
            raise e
          end
          if FEATURE_NOT_SUPPORTED == code
            @statements.delete sql_key(sql)
            retry
          else
            raise e
          end
        end

        # Returns the statement identifier for the client side cache
        # of statements
        def sql_key(sql)
          "#{schema_search_path}-#{sql}"
        end

        # Prepare the statement if it hasn't been prepared, return
        # the statement key.
        def prepare_statement(sql)
          sql_key = sql_key(sql)
          unless @statements.key? sql_key
            nextkey = @statements.next_key
            begin
              @connection.prepare nextkey, sql
            rescue => e
              raise translate_exception_class(e, sql)
            end
            # Clear the queue
            @connection.get_last_result
            @statements[sql_key] = nextkey
          end
          @statements[sql_key]
        end

        # Connects to a PostgreSQL server and sets up the adapter depending on the
        # connected server's characteristics.
        def connect
          @connection = PGconn.connect(@connection_parameters)

          # Money type has a fixed precision of 10 in PostgreSQL 8.2 and below, and as of
          # PostgreSQL 8.3 it has a fixed precision of 19. PostgreSQLColumn.extract_precision
          # should know about this but can't detect it there, so deal with it here.
          OID::Money.precision = (postgresql_version >= 80300) ? 19 : 10

          configure_connection
        rescue ::PG::Error => error
          if error.message.include?("does not exist")
            raise ActiveRecord::NoDatabaseError.new(error.message, error)
          else
            raise
          end
        end

        # Configures the encoding, verbosity, schema search path, and time zone of the connection.
        # This is called by #connect and should not be called manually.
        def configure_connection
          if @config[:encoding]
            @connection.set_client_encoding(@config[:encoding])
          end
          self.client_min_messages = @config[:min_messages] || 'warning'
          self.schema_search_path = @config[:schema_search_path] || @config[:schema_order]

          # Use standard-conforming strings if available so we don't have to do the E'...' dance.
          set_standard_conforming_strings

          # If using Active Record's time zone support configure the connection to return
          # TIMESTAMP WITH ZONE types in UTC.
          # (SET TIME ZONE does not use an equals sign like other SET variables)
          if ActiveRecord::Base.default_timezone == :utc
            execute("SET time zone 'UTC'", 'SCHEMA')
          elsif @local_tz
            execute("SET time zone '#{@local_tz}'", 'SCHEMA')
          end

          # SET statements from :variables config hash
          # http://www.postgresql.org/docs/8.3/static/sql-set.html
          variables = @config[:variables] || {}
          variables.map do |k, v|
            if v == ':default' || v == :default
              # Sets the value to the global or compile default
              execute("SET SESSION #{k.to_s} TO DEFAULT", 'SCHEMA')
            elsif !v.nil?
              execute("SET SESSION #{k.to_s} TO #{quote(v)}", 'SCHEMA')
            end
          end
        end

        # Returns the current ID of a table's sequence.
        def last_insert_id(sequence_name) #:nodoc:
          Integer(last_insert_id_value(sequence_name))
        end

        def last_insert_id_value(sequence_name)
          last_insert_id_result(sequence_name).rows.first.first
        end

        def last_insert_id_result(sequence_name) #:nodoc:
          exec_query("SELECT currval('#{sequence_name}')", 'SQL')
        end

        # Executes a SELECT query and returns the results, performing any data type
        # conversions that are required to be performed here instead of in PostgreSQLColumn.
        def select(sql, name = nil, binds = [])
          exec_query(sql, name, binds)
        end

        # Returns the list of a table's column names, data types, and default values.
        #
        # The underlying query is roughly:
        #  SELECT column.name, column.type, default.value
        #    FROM column LEFT JOIN default
        #      ON column.table_id = default.table_id
        #     AND column.num = default.column_num
        #   WHERE column.table_id = get_table_id('table_name')
        #     AND column.num > 0
        #     AND NOT column.is_dropped
        #   ORDER BY column.num
        #
        # If the table name is not prefixed with a schema, the database will
        # take the first match from the schema search path.
        #
        # Query implementation notes:
        #  - format_type includes the column size constraint, e.g. varchar(50)
        #  - ::regclass is a function that gives the id for a table name
        def column_definitions(table_name) # :nodoc:
          exec_query(<<-end_sql, 'SCHEMA').rows
              SELECT a.attname, format_type(a.atttypid, a.atttypmod),
                     pg_get_expr(d.adbin, d.adrelid), a.attnotnull, a.atttypid, a.atttypmod
                FROM pg_attribute a LEFT JOIN pg_attrdef d
                  ON a.attrelid = d.adrelid AND a.attnum = d.adnum
               WHERE a.attrelid = '#{quote_table_name(table_name)}'::regclass
                 AND a.attnum > 0 AND NOT a.attisdropped
               ORDER BY a.attnum
          end_sql
        end

        def extract_table_ref_from_insert_sql(sql) # :nodoc:
          sql[/into\s+([^\(]*).*values\s*\(/im]
          $1.strip if $1
        end

        def create_table_definition(name, temporary, options, as = nil) # :nodoc:
          PostgreSQL::TableDefinition.new native_database_types, name, temporary, options, as
        end
    end
  end
end